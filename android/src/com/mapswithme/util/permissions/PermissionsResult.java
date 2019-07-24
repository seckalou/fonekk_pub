package com.mapswithme.util.permissions;

public final class PermissionsResult
{
  private final boolean mExternalStorageGranted;
  private final boolean mLocationGranted;
  private final boolean mPhoneStateGranted;

  public PermissionsResult(boolean externalStorageGranted, boolean locationGranted, boolean phoneStateGranted)
  {
    mExternalStorageGranted = externalStorageGranted;
    mLocationGranted = locationGranted;
    mPhoneStateGranted = phoneStateGranted;
  }

  public boolean isExternalStorageGranted()
  {
    return mExternalStorageGranted;
  }

  public boolean isLocationGranted()
  {
    return mLocationGranted;
  }

  public boolean isPhoneStateGranted() { return mPhoneStateGranted; }
}
