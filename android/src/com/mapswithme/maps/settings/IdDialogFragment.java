package com.mapswithme.maps.settings;

import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.TextView;

import com.mapswithme.maps.BuildConfig;
import com.mapswithme.maps.Framework;
import com.mapswithme.maps.R;
import com.mapswithme.maps.base.BaseMwmDialogFragment;
import com.mapswithme.maps.news.BaseNewsFragment;
import com.mapswithme.util.Counters;
import com.mapswithme.util.UiUtils;

public class IdDialogFragment extends BaseMwmDialogFragment implements View.OnClickListener
{
  @Nullable
  private PolicyAgreementListener mListener;

  public static void show(@NonNull FragmentActivity activity)
  {
    create(activity);
    Counters.setFirstStartDialogSeen();
  }

  public static boolean isFirstLaunch(@NonNull FragmentActivity activity)
  {
    if (Counters.getFirstInstallVersion() < BuildConfig.VERSION_CODE)
      return false;

    FragmentManager fm = activity.getSupportFragmentManager();
    if (fm.isDestroyed())
      return false;

    return !Counters.isFirstStartDialogSeen();
  }

  private static void create(@NonNull FragmentActivity activity)
  {
    final IdDialogFragment fragment = new IdDialogFragment();
    activity.getSupportFragmentManager()
            .beginTransaction()
            .add(fragment, IdDialogFragment.class.getName())
            .commitAllowingStateLoss();
  }

  public static boolean recreate(@NonNull FragmentActivity activity)
  {
    FragmentManager fm = activity.getSupportFragmentManager();
    Fragment f = fm.findFragmentByTag(IdDialogFragment.class.getName());
    if (f == null)
      return false;

    // If we're here, it means that the user has rotated the screen.
    // We use different dialog themes for landscape and portrait modes on tablets,
    // so the fragment should be recreated to be displayed correctly.
    fm.beginTransaction()
      .remove(f)
      .commitAllowingStateLoss();
    fm.executePendingTransactions();
    return true;
  }

  @Override
  public void onAttach(Activity activity)
  {
    super.onAttach(activity);
//    if (activity instanceof BaseNewsFragment.NewsDialogListener)
//      mListener = (PolicyAgreementListener) activity;
  }

  @Override
  public void onDetach()
  {
    mListener = null;
    super.onDetach();
  }

  @Override
  protected int getCustomTheme()
  {
    return getFullscreenTheme();
  }

  @NonNull
  @Override
  public Dialog onCreateDialog(Bundle savedInstanceState)
  {
    Dialog res = super.onCreateDialog(savedInstanceState);
    res.requestWindowFeature(Window.FEATURE_NO_TITLE);
    res.setCancelable(false);

    View content = View.inflate(getActivity(), R.layout.fragment_id_dialog, null);
    res.setContentView(content);
    content.findViewById(R.id.btn__id_accept).setOnClickListener(this);
    content.findViewById(R.id.btn__id_exit).setOnClickListener(this);

//    TextView subtitle = content.findViewById(R.id.tv__subtitle1);
//    subtitle.setText(R.string.onboarding_welcome_first_subtitle);


    return res;
  }

  @Override
  public void onClick(View v)
  {
    switch (v.getId())
    {
      case R.id.btn__id_exit:
        getActivity().finish();
        break;

      case R.id.btn__id_accept:
        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        Uri uri = Uri.fromParts("package", getContext().getPackageName(), null);
        intent.setData(uri);
        startActivity(intent);
        break;
    }

//    dismiss();
  }

  @Override
  public void onCancel(DialogInterface dialog)
  {
    super.onCancel(dialog);
    requireActivity().finish();
  }

  public interface PolicyAgreementListener
  {
    void onPolicyAgreementApplied();
  }
}
