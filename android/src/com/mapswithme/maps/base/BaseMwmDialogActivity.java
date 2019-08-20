package com.mapswithme.maps.base;

import android.app.Activity;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Bundle;
import android.support.annotation.CallSuper;
import android.support.annotation.ColorRes;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.StyleRes;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;

import com.mapswithme.maps.MwmApplication;
import com.mapswithme.maps.R;
import com.mapswithme.maps.SplashActivity;
import com.mapswithme.util.Config;
import com.mapswithme.util.PermissionsUtils;
import com.mapswithme.util.ThemeUtils;
import com.mapswithme.util.UiUtils;
import com.mapswithme.util.Utils;
import com.mapswithme.util.log.Logger;
import com.mapswithme.util.log.LoggerFactory;

public abstract class BaseMwmDialogActivity extends AppCompatActivity
                                  implements BaseActivity
{

    @Override
    @NonNull
    public Activity get()
    {
        return this;
    }

    @Override
    @StyleRes
    public int getThemeResourceId(@NonNull String theme)
    {
        if (ThemeUtils.isDefaultTheme(theme))
            return R.style.MwmTheme;

        if (ThemeUtils.isNightTheme(theme))
            return R.style.MwmTheme_Night;

        throw new IllegalArgumentException("Attempt to apply unsupported theme: " + theme);
    }

    protected Class<? extends Fragment> getFragmentClass()
    {
        return null;
    }


    @Override
    public void onBackPressed()
    {
        if (getFragmentClass() == null)
        {
            super.onBackPressed();
            return;
        }
        FragmentManager manager = getSupportFragmentManager();
        String name = getFragmentClass().getName();
        Fragment fragment = manager.findFragmentByTag(name);

        if (fragment == null)
        {
            super.onBackPressed();
            return;
        }

        super.onBackPressed();
    }
}
