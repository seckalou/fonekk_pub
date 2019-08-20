package com.mapswithme.maps.permissions;

import android.content.DialogInterface;
import android.content.Intent;
import android.support.annotation.DrawableRes;
import android.support.annotation.IdRes;
import android.support.annotation.LayoutRes;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.StringRes;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.widget.TextView;
import android.view.View;
import com.mapswithme.maps.R;
import com.mapswithme.maps.auth.Authorizer;
import com.mapswithme.maps.auth.TargetFragmentCallback;
import com.mapswithme.maps.bookmarks.data.BookmarkCategory;
import android.view.View.OnFocusChangeListener;

public class PermissionsDialogFragment extends BasePermissionsDialogFragment implements TargetFragmentCallback

{
  @Nullable
  public static DialogFragment show(@NonNull FragmentActivity activity, int requestCode)
  {
    return BasePermissionsDialogFragment.show(activity, requestCode, PermissionsDialogFragment.class);
  }

  public static DialogFragment find(@NonNull FragmentActivity activity)
  {
    final FragmentManager fm = activity.getSupportFragmentManager();
    if (fm.isDestroyed())
      return null;

    Fragment f = fm.findFragmentByTag(PermissionsDialogFragment.class.getName());
    return (DialogFragment) f;
  }

  @DrawableRes
  @Override
  protected int getImageRes()
  {
    return R.drawable.img_permissions;
  }

  @StringRes
  @Override
  protected int getTitleRes()
  {
    return R.string.onboarding_permissions_title;
  }

  @StringRes
  @Override
  protected int getSubtitleRes()
  {
    return R.string.onboarding_setting_message;
  }

  @LayoutRes
  @Override
  protected int getLayoutRes()
  {
    return R.layout.fragment_permissions;
  }

  @IdRes
  @Override
  protected int getFirstActionButton()
  {
    return R.id.btn__learn_more;
  }

  @Override
  protected void onFirstActionClick()
  {
    PermissionsDetailDialogFragment.show(getActivity(), getRequestCode());
  }

  @IdRes
  @Override
  protected int getContinueActionButton()
  {
    return R.id.btn__continue;
  }

  @Override
  public void dismiss()
  {
    DialogFragment dialog = PermissionsDetailDialogFragment.find(getActivity());
    if (dialog != null)
      dialog.dismiss();
    super.dismiss();
  }

  @Override
  public void onCancel(DialogInterface dialog)
  {
    super.onCancel(dialog);
    getActivity().finish();
  }

  @Override
  public void onTargetFragmentResult(int resultCode, @Nullable Intent data) {
  }

  @Override
  public boolean isTargetAdded() {
    return false;
  }


  @Override
  public  void onResume(){
    super.onResume();

//    showSubtitle(true);

    if (isPermissionGranted() && !isAuthorizationGranted())
      mAuthorizer.authorize();

  }
}
