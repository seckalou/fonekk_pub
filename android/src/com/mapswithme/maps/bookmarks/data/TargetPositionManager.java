package com.mapswithme.maps.bookmarks.data;

import android.support.annotation.NonNull;

import com.mapswithme.maps.bookmarks.TargetPositionListener;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public enum TargetPositionManager {
    INSTANCE;

    private final BlockingQueue<TargetPosition> mTargetPositions = new LinkedBlockingQueue<>();
    private final PositionConsumer mPositionConsumer = new PositionConsumer(mTargetPositions);

    @NonNull
    private final List<TargetPositionListener> mListeners = new ArrayList<>();

    class PositionConsumer implements Runnable{

        private final BlockingQueue<TargetPosition> queue;

        PositionConsumer(BlockingQueue<TargetPosition> q) { queue = q; }

        public void run() {
            try {
                while (true) { consume(queue.take()); }
            } catch (InterruptedException ex) { System.err.println("An error occured while reading targetPositions");}
        }

        void consume(TargetPosition tp) {
            for (TargetPositionListener l: mListeners) {
                l.onNewTargetPositionAvailable(tp);
            }
        }
    }

    public void addTargetPositionListener(TargetPositionListener tpl){
        mListeners.add(tpl);
    }
    public void produceTargetPosition(float lon, float lat){
        TargetPosition tp = new TargetPosition(lon, lat);

        try{
            mTargetPositions.put(tp);
        }catch (InterruptedException ex) { System.err.println("An error occured while creating targetPositions");}
    }
}
