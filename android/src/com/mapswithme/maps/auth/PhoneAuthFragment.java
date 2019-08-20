package com.mapswithme.maps.auth;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import com.mapswithme.maps.Framework;
import com.mapswithme.maps.R;
import com.mapswithme.maps.base.BaseMwmDialogFragment;
import com.mapswithme.util.UiUtils;

public class PhoneAuthFragment extends BaseWebViewMwmFragment
{
  private static final String REDIRECT_URL = "http://localhost";

  @SuppressWarnings("NullableProblems")
  @NonNull
  private WebView mWebView;
  @SuppressWarnings("NullableProblems")
  @NonNull
  private View mProgress;

  @Nullable
  @Override
  public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                           @Nullable Bundle savedInstanceState)
  {
    return inflater.inflate(R.layout.fragment_web_view_with_progress, container, false);
  }

  @SuppressLint("SetJavaScriptEnabled")
  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState)
  {
    super.onViewCreated(view, savedInstanceState);

    mWebView = view.findViewById(getWebViewResId());
    mProgress = view.findViewById(R.id.progress);
    mWebView.setWebViewClient(new WebViewClient()
    {
      @Override
      public void onPageFinished(WebView view, String url)
      {
        UiUtils.show(mWebView);
        UiUtils.hide(mProgress);

        FinalAuthFragment.show(PhoneAuthFragment.this);
      }

      @Override
      public boolean shouldOverrideUrlLoading(WebView view, String url)
      {
        if (!TextUtils.isEmpty(url) && url.contains(REDIRECT_URL + "/?code="))
        {
          Intent returnIntent = new Intent();
          returnIntent.putExtra(Constants.EXTRA_PHONE_AUTH_TOKEN,
                                url.substring((REDIRECT_URL + "/?code=").length()));

          getActivity().setResult(Activity.RESULT_OK, returnIntent);
          getActivity().finish();

          return true;
        }

        return super.shouldOverrideUrlLoading(view, url);
      }
    });

    mWebView.getSettings().setJavaScriptEnabled(true);

    String tmp = Framework.nativeGetUserAgent();
    mWebView.getSettings().setUserAgentString(tmp);
    tmp = Framework.nativeGetPhoneAuthUrl(REDIRECT_URL);
    mWebView.loadUrl("http://alassaneseckcom.ipage.com/fonekk/auth.php");
//    mWebView.loadUrl("http://google.com");
  }

  static public class FinalAuthFragment extends BaseMwmDialogFragment{

    public  static void show(Fragment parent){
      String name = FinalAuthFragment.class.getName();
      DialogFragment fragment = (DialogFragment) parent.getChildFragmentManager()
              .findFragmentByTag(name);
      if (fragment != null)
        return;

      fragment = (DialogFragment) Fragment.instantiate(parent.getContext(), name);
      fragment.show(parent.getChildFragmentManager(), name);
    }
    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState)
    {
      Dialog res = super.onCreateDialog(savedInstanceState);
      res.requestWindowFeature(Window.FEATURE_NO_TITLE);
      return res;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState)
    {
      View view = inflater.inflate(R.layout.fragment_final_auth_dialog, container, false);
//
//      setLoginButton(view, R.id.google_button, mGoogleClickListener);
//      setLoginButton(view, R.id.facebook_button, mFacebookClickListener);
//      setLoginButton(view, R.id.phone_button, mPhoneClickListener);
//
////    mPromoCheck = view.findViewById(R.id.newsCheck);
//      mPrivacyPolicyCheck = view.findViewById(R.id.privacyPolicyCheck);
//      mPrivacyPolicyCheck.setOnCheckedChangeListener((buttonView, isChecked) -> {
//        setButtonAvailability(view, isChecked && mTermOfUseCheck.isChecked(),
//                R.id.google_button, R.id.facebook_button, R.id.phone_button);
//      });
//
//      mTermOfUseCheck = view.findViewById(R.id.termOfUseCheck);
//      mTermOfUseCheck.setOnCheckedChangeListener((buttonView, isChecked) -> {
//        setButtonAvailability(view, isChecked && mPrivacyPolicyCheck.isChecked(),
//                R.id.google_button, R.id.facebook_button, R.id.phone_button);
//      });
//
//      linkifyPolicyView(view, R.id.privacyPolicyLink, R.string.sign_agree_pp_gdpr,
//              Framework.nativeGetPrivacyPolicyLink());
//
//      linkifyPolicyView(view, R.id.termOfUseLink, R.string.sign_agree_tof_gdpr,
//              Framework.nativeGetTermsOfUseLink());
//
//      setButtonAvailability(view, false, R.id.google_button, R.id.facebook_button,
//              R.id.phone_button);
      return view;
    }

  }
}
