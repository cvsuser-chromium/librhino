package com.hybroad.image;

import android.graphics.drawable.Drawable;

public interface ImageCallback {
    public void imageLoaded(Drawable imageDrawable, String imageTag);
}
