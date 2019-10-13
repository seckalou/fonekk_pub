package com.mapswithme.maps.bookmarks;

import com.mapswithme.maps.bookmarks.data.TargetPosition;

public interface TargetPositionListener {
    public void onNewTargetPositionAvailable(TargetPosition pos);
}
