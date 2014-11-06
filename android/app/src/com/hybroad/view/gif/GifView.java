package com.hybroad.view.gif;

import java.io.InputStream;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.ImageView;

/**
 * Show .jpg .png .9.path .gif
 * 
 * @author star
 * @since 2013/8/14
 */
public class GifView extends ImageView implements GifAction {
  private static final String TAG = "GifView";
  private GifDecoder gifDecoder = null;
  private Bitmap currentImage = null;
  private boolean isRun = true;
  private boolean pause = false;
  private int showWidth = -1;
  private int showHeight = -1;
  private Rect rect = null;
  private DrawThread drawThread = null;
  private GifImageType animationType = GifImageType.SYNC_DECODER;

  public GifView(Context context, AttributeSet attrs, int defStyle) {
    super(context, attrs, defStyle);
  }

  public GifView(Context context, AttributeSet attrs) {
    super(context, attrs);
  }

  public GifView(Context context) {
    super(context);
  }

  public enum GifImageType {
    WAIT_FINISH(0), SYNC_DECODER(1), COVER(2);

    GifImageType(int i) {
      nativeInt = i;
    }

    final int nativeInt;
  }

  public void setDrawThreadRunFlag(boolean run) {
    synchronized (this) {
      isRun = run;
    }
  }

  /**
   * This method is called before 'setGifImage'.
   * 
   */
  public void setGifImageType(GifImageType type) {
    if (gifDecoder == null) {
      animationType = type;
    }

  }

  public void setShowDimension(int width, int height) {
    if (width > 0 && height > 0) {
      showWidth = width;
      showHeight = height;
      rect = new Rect();
      rect.left = 0;
      rect.top = 0;
      rect.right = width;
      rect.bottom = height;
    }
  }

  private void setGifDecoderImage(byte[] gif) {
    if (gifDecoder != null) {
      gifDecoder.free();
      gifDecoder = null;
    }
    gifDecoder = new GifDecoder(gif, this);
    gifDecoder.start();
  }

  private void setGifDecoderImage(InputStream is) {
    if (gifDecoder != null) {
      gifDecoder.free();
      gifDecoder = null;
    }
    gifDecoder = new GifDecoder(is, this);
    gifDecoder.start();
  }

  public void setGifImage(byte[] gif, boolean showAnimation) {
    if (showAnimation) {
      pause = false;
      Log.d(TAG, "setGifImage: showAnimation = " + showAnimation);
      setGifDecoderImage(gif);
    } else {
      Log.d(TAG, "setGifImage: showAnimation = " + showAnimation);
      showCover(gif);
    }
  }

  public void setGifImage(InputStream is, boolean showAnimation) {
    if (showAnimation) {
      pause = false;
      setGifDecoderImage(is);
    } else {
      showCover(is);
    }
  }

  private void showCover(InputStream is) {
    if (gifDecoder == null)
      setGifDecoderImage(is);

    pause = true;
    currentImage = gifDecoder.getImage();
    // setImageBitmap(currentImage);
    invalidate();
  }

  private void showCover(byte[] gif) {
    if (gifDecoder == null)
      setGifDecoderImage(gif);

    pause = true;
    currentImage = gifDecoder.getImage();
    // setImageBitmap(currentImage);
    invalidate();
  }

  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    if (gifDecoder == null)
      return;
    if (currentImage == null) {
      currentImage = gifDecoder.getImage();
    }
    if (currentImage == null) {
      return;
    }
    int saveCount = canvas.getSaveCount();
    canvas.save();
    canvas.translate(getPaddingLeft(), getPaddingTop());
    if (showWidth == -1 || showHeight == -1) {
      canvas.drawBitmap(currentImage, 0, 0, null);
    } else {
      canvas.drawBitmap(currentImage, null, rect, null);
    }
    canvas.restoreToCount(saveCount);
  }

  protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int pleft = getPaddingLeft();
    int pright = getPaddingRight();
    int ptop = getPaddingTop();
    int pbottom = getPaddingBottom();

    int widthSize;
    int heightSize;

    int w = 100;
    int h = 100;

    if (gifDecoder == null) {
      Drawable d = getDrawable();
      if (null == d) {
        w = 0;
        h = 0;
      } else {
        w = d.getIntrinsicWidth();
        h = d.getIntrinsicHeight();
      }
    } else {
      w = gifDecoder.width;
      h = gifDecoder.height;
    }

    w += pleft + pright;
    h += ptop + pbottom;

    w = Math.max(w, getSuggestedMinimumWidth());
    h = Math.max(h, getSuggestedMinimumHeight());

    widthSize = resolveSize(w, widthMeasureSpec);
    heightSize = resolveSize(h, heightMeasureSpec);

    setMeasuredDimension(widthSize, heightSize);
  }

  public void parseOk(boolean parseStatus, int frameIndex) {
    if (parseStatus) {
      if (gifDecoder != null) {
        switch (animationType) {
        case WAIT_FINISH:
          if (frameIndex == -1) {
            if (gifDecoder.getFrameCount() > 1) {
              DrawThread dt = new DrawThread();
              dt.start();
            } else {
              reDraw();
            }
          }
          break;
        case COVER:
          if (frameIndex == 1) {
            currentImage = gifDecoder.getImage();
            reDraw();
          } else if (frameIndex == -1) {
            if (gifDecoder.getFrameCount() > 1) {
              if (drawThread == null) {
                drawThread = new DrawThread();
                drawThread.start();
              }
            } else {
              reDraw();
            }
          }
          break;
        case SYNC_DECODER:
          if (frameIndex == 1) {
            currentImage = gifDecoder.getImage();
            reDraw();
          } else if (frameIndex == -1) {
            reDraw();
          } else {
            if (drawThread == null) {
              drawThread = new DrawThread();
              drawThread.start();
            }
          }
          break;
        }

      } else {
        Log.e("gif", "parse error");
      }

    }
  }

  private void reDraw() {
    if (redrawHandler != null) {
      Message msg = redrawHandler.obtainMessage();
      redrawHandler.sendMessage(msg);
    }
  }

  private Handler redrawHandler = new Handler() {
    public void handleMessage(Message msg) {
      invalidate();
    }
  };

  private class DrawThread extends Thread {
    public void run() {
      if (gifDecoder == null) {
        return;
      }
      while (isRun) {
        if (!pause) {
          // if(gifDecoder.parseOk()){
          GifFrame frame = gifDecoder.next();
          currentImage = frame.image;
          long sp = frame.delay;
          if (redrawHandler != null) {
            Message msg = redrawHandler.obtainMessage();
            redrawHandler.sendMessage(msg);
            SystemClock.sleep(sp);
          } else {
            break;
          }
        } else {
          SystemClock.sleep(10);
        }
      }
    }
  }

}
