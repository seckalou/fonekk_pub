#include "map/bookmark_manager.hpp"
#include "map/api_mark_point.hpp"
#include "map/local_ads_mark.hpp"
#include "map/routing_mark.hpp"
#include "map/search_mark.hpp"
#include "map/user_mark.hpp"

#include "drape_frontend/drape_engine.hpp"
#include "drape_frontend/visual_params.hpp"

#include "platform/platform.hpp"
#include "platform/settings.hpp"

#include "indexer/scales.hpp"

#include "coding/file_name_utils.hpp"
#include "coding/file_writer.hpp"
#include "coding/internal/file_data.hpp"
#include "coding/zip_reader.hpp"

#include "geometry/transformations.hpp"

#include "base/macros.hpp"
#include "base/stl_add.hpp"

#include "std/target_os.hpp"

#include <algorithm>

namespace
{
char const * BOOKMARK_CATEGORY = "LastBookmarkCategory";
char const * BOOKMARK_TYPE = "LastBookmarkType";
char const * KMZ_EXTENSION = ".kmz";

using SearchUserMarkContainer = SpecifiedUserMarkContainer<SearchMarkPoint, UserMark::Type::SEARCH>;
using ApiUserMarkContainer = SpecifiedUserMarkContainer<ApiMarkPoint, UserMark::Type::API>;
using DebugUserMarkContainer = SpecifiedUserMarkContainer<DebugMarkPoint, UserMark::Type::DEBUG_MARK>;
using RouteUserMarkContainer = SpecifiedUserMarkContainer<RouteMarkPoint, UserMark::Type::ROUTING>;
using LocalAdsMarkContainer = SpecifiedUserMarkContainer<LocalAdsMark, UserMark::Type::LOCAL_ADS>;
using TransitMarkContainer = SpecifiedUserMarkContainer<TransitMark, UserMark::Type::TRANSIT>;
using StaticUserMarkContainer = SpecifiedUserMarkContainer<SearchMarkPoint, UserMark::Type::STATIC>;

// Returns extension with a dot in a lower case.
std::string const GetFileExt(std::string const & filePath)
{
  std::string ext = my::GetFileExtension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext;
}

std::string const GetFileName(std::string const & filePath)
{
  std::string ret = filePath;
  my::GetNameFromFullPath(ret);
  return ret;
}

std::string const GenerateValidAndUniqueFilePathForKML(std::string const & fileName)
{
  std::string filePath = BookmarkCategory::RemoveInvalidSymbols(fileName);
  filePath = BookmarkCategory::GenerateUniqueFileName(GetPlatform().SettingsDir(), filePath);
  return filePath;
}
}  // namespace

BookmarkManager::BookmarkManager(GetStringsBundleFn && getStringsBundleFn)
  : m_getStringsBundle(std::move(getStringsBundleFn))
  , m_needTeardown(false)
{
  ASSERT(m_getStringsBundle != nullptr, ());

  m_userMarkLayers.emplace_back(my::make_unique<SearchUserMarkContainer>());
  m_userMarkLayers.emplace_back(my::make_unique<ApiUserMarkContainer>());
  m_userMarkLayers.emplace_back(my::make_unique<DebugUserMarkContainer>());
  m_userMarkLayers.emplace_back(my::make_unique<RouteUserMarkContainer>());
  m_userMarkLayers.emplace_back(my::make_unique<LocalAdsMarkContainer>());
  m_userMarkLayers.emplace_back(my::make_unique<TransitMarkContainer>());

  auto staticMarksContainer = my::make_unique<StaticUserMarkContainer>();
  m_selectionMark = my::make_unique<StaticMarkPoint>(staticMarksContainer.get());
  m_myPositionMark = my::make_unique<MyPositionMarkPoint>(staticMarksContainer.get());

  m_userMarkLayers.emplace_back(std::move(staticMarksContainer));
}

BookmarkManager::~BookmarkManager()
{
  m_userMarkLayers.clear();

  ClearCategories();
}

void BookmarkManager::SetDrapeEngine(ref_ptr<df::DrapeEngine> engine)
{
  m_drapeEngine.Set(engine);

  for (auto & userMarkLayer : m_userMarkLayers)
    userMarkLayer->SetDrapeEngine(engine);

  for (auto & category : m_categories)
    category->SetDrapeEngine(engine);
}

void BookmarkManager::UpdateViewport(ScreenBase const & screen)
{
  m_viewport = screen;
}

void BookmarkManager::SetAsyncLoadingCallbacks(AsyncLoadingCallbacks && callbacks)
{
  m_asyncLoadingCallbacks = std::move(callbacks);
}

void BookmarkManager::Teardown()
{
  m_needTeardown = true;
}

void BookmarkManager::SaveState() const
{
  settings::Set(BOOKMARK_CATEGORY, m_lastCategoryUrl);
  settings::Set(BOOKMARK_TYPE, m_lastType);
}

void BookmarkManager::LoadState()
{
  UNUSED_VALUE(settings::Get(BOOKMARK_CATEGORY, m_lastCategoryUrl));
  UNUSED_VALUE(settings::Get(BOOKMARK_TYPE, m_lastType));
}

void BookmarkManager::ClearCategories()
{
  m_categories.clear();
}

void BookmarkManager::LoadBookmarks()
{
  ClearCategories();

  NotifyAboutStartAsyncLoading();
  GetPlatform().RunTask(Platform::Thread::File, [this]()
  {
    std::string const dir = GetPlatform().SettingsDir();
    Platform::FilesList files;
    Platform::GetFilesByExt(dir, BOOKMARKS_FILE_EXTENSION, files);

    auto collection = std::make_shared<CategoriesCollection>();
    for (auto const & file : files)
    {
      auto cat = BookmarkCategory::CreateFromKMLFile(dir + file);
      if (m_needTeardown)
        return;

      if (cat != nullptr)
        collection->emplace_back(std::move(cat));
    }
    NotifyAboutFinishAsyncLoading(std::move(collection));
  });

  LoadState();
}

void BookmarkManager::LoadBookmark(std::string const & filePath, bool isTemporaryFile)
{
  NotifyAboutStartAsyncLoading();
  GetPlatform().RunTask(Platform::Thread::File, [this, filePath, isTemporaryFile]()
  {
    auto collection = std::make_shared<CategoriesCollection>();
    auto const fileSavePath = GetKMLPath(filePath);
    if (m_needTeardown)
      return;

    if (!fileSavePath)
    {
      NotifyAboutFile(false /* success */, filePath, isTemporaryFile);
    }
    else
    {
      auto cat = BookmarkCategory::CreateFromKMLFile(fileSavePath.get());
      if (m_needTeardown)
        return;

      bool const categoryExists = (cat != nullptr);
      if (categoryExists)
        collection->emplace_back(std::move(cat));

      NotifyAboutFile(categoryExists, filePath, isTemporaryFile);
    }
    NotifyAboutFinishAsyncLoading(std::move(collection));
  });
}

void BookmarkManager::NotifyAboutStartAsyncLoading()
{
  if (m_needTeardown)
    return;
  
  GetPlatform().RunTask(Platform::Thread::Gui, [this]()
  {
    m_asyncLoadingCounter++;
    if (m_asyncLoadingCallbacks.m_onStarted != nullptr)
      m_asyncLoadingCallbacks.m_onStarted();
  });
}

void BookmarkManager::NotifyAboutFinishAsyncLoading(std::shared_ptr<CategoriesCollection> && collection)
{
  if (m_needTeardown)
    return;
  
  GetPlatform().RunTask(Platform::Thread::Gui, [this, collection]()
  {
    if (!collection->empty())
      MergeCategories(std::move(*collection));

    m_asyncLoadingCounter--;
    if (m_asyncLoadingCounter == 0 && m_asyncLoadingCallbacks.m_onFinished != nullptr)
      m_asyncLoadingCallbacks.m_onFinished();
  });
}

void BookmarkManager::NotifyAboutFile(bool success, std::string const & filePath,
                                      bool isTemporaryFile)
{
  if (m_needTeardown)
    return;
  
  GetPlatform().RunTask(Platform::Thread::Gui, [this, success, filePath, isTemporaryFile]()
  {
    if (success)
    {
      if (m_asyncLoadingCallbacks.m_onFileSuccess != nullptr)
        m_asyncLoadingCallbacks.m_onFileSuccess(filePath, isTemporaryFile);
    }
    else
    {
      if (m_asyncLoadingCallbacks.m_onFileError != nullptr)
        m_asyncLoadingCallbacks.m_onFileError(filePath, isTemporaryFile);
    }
  });
}

boost::optional<std::string> BookmarkManager::GetKMLPath(std::string const & filePath)
{
  std::string const fileExt = GetFileExt(filePath);
  string fileSavePath;
  if (fileExt == BOOKMARKS_FILE_EXTENSION)
  {
    fileSavePath = GenerateValidAndUniqueFilePathForKML(GetFileName(filePath));
    if (!my::CopyFileX(filePath, fileSavePath))
      return {};
  }
  else if (fileExt == KMZ_EXTENSION)
  {
    try
    {
      ZipFileReader::FileListT files;
      ZipFileReader::FilesList(filePath, files);
      std::string kmlFileName;
      for (size_t i = 0; i < files.size(); ++i)
      {
        if (GetFileExt(files[i].first) == BOOKMARKS_FILE_EXTENSION)
        {
          kmlFileName = files[i].first;
          break;
        }
      }
      if (kmlFileName.empty())
        return {};

      fileSavePath = GenerateValidAndUniqueFilePathForKML(kmlFileName);
      ZipFileReader::UnzipFile(filePath, kmlFileName, fileSavePath);
    }
    catch (RootException const & e)
    {
      LOG(LWARNING, ("Error unzipping file", filePath, e.Msg()));
      return {};
    }
  }
  else
  {
    LOG(LWARNING, ("Unknown file type", filePath));
    return {};
  }
  return fileSavePath;
}

void BookmarkManager::InitBookmarks()
{
  for (auto & cat : m_categories)
    cat->NotifyChanges();
}

size_t BookmarkManager::AddBookmark(size_t categoryIndex, m2::PointD const & ptOrg, BookmarkData & bm)
{
  bm.SetTimeStamp(time(0));
  bm.SetScale(df::GetDrawTileScale(m_viewport));

  BookmarkCategory & cat = *m_categories[categoryIndex];

  auto bookmark = static_cast<Bookmark *>(cat.CreateUserMark(ptOrg));
  bookmark->SetData(bm);
  cat.SetIsVisible(true);
  cat.SaveToKMLFile();
  cat.NotifyChanges();

  m_lastCategoryUrl = cat.GetFileName();
  m_lastType = bm.GetType();
  SaveState();

  // Bookmark always is pushed front.
  return 0;
}

size_t BookmarkManager::MoveBookmark(size_t bmIndex, size_t curCatIndex, size_t newCatIndex)
{
  BookmarkData data;
  m2::PointD ptOrg;

  BookmarkCategory * cat = GetBmCategory(curCatIndex);
  auto bm = static_cast<Bookmark const *>(cat->GetUserMark(bmIndex));
  data = bm->GetData();
  ptOrg = bm->GetPivot();

  cat->DeleteUserMark(bmIndex);
  cat->SaveToKMLFile();
  cat->NotifyChanges();

  return AddBookmark(newCatIndex, ptOrg, data);
}

void BookmarkManager::ReplaceBookmark(size_t catIndex, size_t bmIndex, BookmarkData const & bm)
{
  BookmarkCategory & cat = *m_categories[catIndex];
  static_cast<Bookmark *>(cat.GetUserMarkForEdit(bmIndex))->SetData(bm);
  cat.SaveToKMLFile();
  cat.NotifyChanges();

  m_lastType = bm.GetType();
  SaveState();
}

size_t BookmarkManager::LastEditedBMCategory()
{
  for (size_t i = 0; i < m_categories.size(); ++i)
  {
    if (m_categories[i]->GetFileName() == m_lastCategoryUrl)
      return i;
  }

  if (m_categories.empty())
    CreateBmCategory(m_getStringsBundle().GetString("my_places"));

  return 0;
}

std::string BookmarkManager::LastEditedBMType() const
{
  return (m_lastType.empty() ? BookmarkCategory::GetDefaultType() : m_lastType);
}

BookmarkCategory * BookmarkManager::GetBmCategory(size_t index) const
{
  return (index < m_categories.size() ? m_categories[index].get() : 0);
}

size_t BookmarkManager::CreateBmCategory(std::string const & name)
{
  m_categories.emplace_back(new BookmarkCategory(name));
  df::DrapeEngineLockGuard lock(m_drapeEngine);
  if (lock)
    m_categories.back()->SetDrapeEngine(lock.Get());

  return (m_categories.size() - 1);
}

void BookmarkManager::DeleteBmCategory(CategoryIter it)
{
  BookmarkCategory & cat = *it->get();
  cat.DeleteLater();
  FileWriter::DeleteFileX(cat.GetFileName());
  m_categories.erase(it);
}

bool BookmarkManager::DeleteBmCategory(size_t index)
{
  if (index >= m_categories.size())
    return false;

  DeleteBmCategory(m_categories.begin() + index);
  return true;
}

namespace
{
class BestUserMarkFinder
{
public:
  explicit BestUserMarkFinder(BookmarkManager::TTouchRectHolder const & rectHolder)
    : m_rectHolder(rectHolder)
    , m_d(numeric_limits<double>::max())
    , m_mark(nullptr)
  {}

  void operator()(UserMarkContainer const * container)
  {
    ASSERT(container != nullptr, ());
    m2::AnyRectD const & rect = m_rectHolder(container->GetType());
    if (UserMark const * p = container->FindMarkInRect(rect, m_d))
    {
      static double const kEps = 1e-5;
      if (m_mark == nullptr || !p->GetPivot().EqualDxDy(m_mark->GetPivot(), kEps))
        m_mark = p;
    }
  }

  UserMark const * GetFoundMark() const { return m_mark; }

private:
  BookmarkManager::TTouchRectHolder const & m_rectHolder;
  double m_d;
  UserMark const * m_mark;
};
}  // namespace

UserMark const * BookmarkManager::FindNearestUserMark(m2::AnyRectD const & rect) const
{
  return FindNearestUserMark([&rect](UserMark::Type) { return rect; });
}

UserMark const * BookmarkManager::FindNearestUserMark(TTouchRectHolder const & holder) const
{
  BestUserMarkFinder finder(holder);
  finder(FindUserMarksContainer(UserMark::Type::ROUTING));
  finder(FindUserMarksContainer(UserMark::Type::SEARCH));
  finder(FindUserMarksContainer(UserMark::Type::API));
  for (auto & cat : m_categories)
    finder(cat.get());

  return finder.GetFoundMark();
}

bool BookmarkManager::UserMarksIsVisible(UserMark::Type type) const
{
  return FindUserMarksContainer(type)->IsVisible();
}

UserMarksController & BookmarkManager::GetUserMarksController(UserMark::Type type)
{
  return *FindUserMarksContainer(type);
}

UserMarkContainer const * BookmarkManager::FindUserMarksContainer(UserMark::Type type) const
{
  auto const iter = std::find_if(m_userMarkLayers.begin(), m_userMarkLayers.end(),
                                 [&type](std::unique_ptr<UserMarkContainer> const & cont)
  {
    return cont->GetType() == type;
  });
  ASSERT(iter != m_userMarkLayers.end(), ());
  return iter->get();
}

UserMarkContainer * BookmarkManager::FindUserMarksContainer(UserMark::Type type)
{
  auto iter = std::find_if(m_userMarkLayers.begin(), m_userMarkLayers.end(),
                           [&type](std::unique_ptr<UserMarkContainer> const & cont)
  {
    return cont->GetType() == type;
  });
  ASSERT(iter != m_userMarkLayers.end(), ());
  return iter->get();
}

std::unique_ptr<StaticMarkPoint> & BookmarkManager::SelectionMark()
{
  ASSERT(m_selectionMark != nullptr, ());
  return m_selectionMark;
}

std::unique_ptr<MyPositionMarkPoint> & BookmarkManager::MyPositionMark()
{
  ASSERT(m_myPositionMark != nullptr, ());
  return m_myPositionMark;
}

std::unique_ptr<StaticMarkPoint> const & BookmarkManager::SelectionMark() const
{
  ASSERT(m_selectionMark != nullptr, ());
  return m_selectionMark;
}

std::unique_ptr<MyPositionMarkPoint> const & BookmarkManager::MyPositionMark() const
{
  ASSERT(m_myPositionMark != nullptr, ());
  return m_myPositionMark;
}

void BookmarkManager::MergeCategories(CategoriesCollection && newCategories)
{
  for (auto & category : m_categories)
  {
    // Since all KML-files are being loaded asynchronously user can create
    // new category during loading. So we have to merge categories after loading.
    std::string const categoryName = category->GetName();
    auto const it = std::find_if(newCategories.begin(), newCategories.end(),
                                 [&categoryName](std::unique_ptr<BookmarkCategory> const & c)
    {
      return c->GetName() == categoryName;
    });
    if (it == newCategories.end())
      continue;

    // Copy bookmarks and tracks to the existing category.
    for (size_t i = 0; i < (*it)->GetUserMarkCount(); ++i)
    {
      auto srcBookmark = static_cast<Bookmark const *>((*it)->GetUserMark(i));
      auto bookmark = static_cast<Bookmark *>(category->CreateUserMark(srcBookmark->GetPivot()));
      bookmark->SetData(srcBookmark->GetData());
    }
    category->AppendTracks((*it)->StealTracks());
    category->SaveToKMLFile();

    newCategories.erase(it);
  }

  std::move(newCategories.begin(), newCategories.end(), std::back_inserter(m_categories));

  df::DrapeEngineLockGuard lock(m_drapeEngine);
  if (lock)
  {
    for (auto & cat : m_categories)
    {
      cat->SetDrapeEngine(lock.Get());
      cat->NotifyChanges();
    }
  }
}

UserMarkNotificationGuard::UserMarkNotificationGuard(BookmarkManager & mng, UserMark::Type type)
  : m_controller(mng.GetUserMarksController(type))
{}

UserMarkNotificationGuard::~UserMarkNotificationGuard()
{
  m_controller.NotifyChanges();
}
