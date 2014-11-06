package com.hybroad.image;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import org.apache.http.conn.ConnectTimeoutException;

import com.hybroad.util.Converter;

import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.util.Log;

public class ImageUtils {
	private static final String TAG = "----ImageUtils----";
	private static final int UNCONSTRAINED = -1;
	private static int mThumbTargetWidth = 170;//1024;//256;//512;//1024;
	private static int mThumbTargetHeight = 128;//768;//192;//384;//768;
	
	/** Must be used in the sub-thread */
	public static Bitmap getImageThumbnail(Context context, String imagePath, int targetWidth, int targetHeight) {
		if (context == null || imagePath == null || targetWidth < 0 || targetHeight < 0) {
			return null;
		}
		String transformedPath = Converter.convertSpecialCharacters(imagePath);
		Bitmap bitmap = null;		
		ContentResolver cr = context.getContentResolver();
		//bitmap = MediaStore.Images.Thumbnails.getThumbnail(cr, image_id, MediaStore.Images.Thumbnails.MICRO_KIND, null);
		if (bitmap == null) {
			Bitmap bitmapTemp = null;
			InputStream inputStream = null;
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inSampleSize = 1;
			options.inJustDecodeBounds = true;
			HttpURLConnection httpURLConnection = null;
			URL url = null;
			try {
				Log.d(TAG, "get thumbnail from original image...");
				// First, read the actual size of the image
				if (transformedPath.startsWith("http://")) {
				    url = new URL(transformedPath);
				    httpURLConnection = (HttpURLConnection) url.openConnection();
				    httpURLConnection.setConnectTimeout(5000);
	                httpURLConnection.setReadTimeout(5000);
	                httpURLConnection.setRequestMethod("GET");
	                int responseCode;
	                if (200 == (responseCode = httpURLConnection.getResponseCode())) {
	                    inputStream = httpURLConnection.getInputStream();                    
	                } else {
	                    Log.e(TAG, "connect failure! responseCode: " + responseCode);
	                    if (httpURLConnection != null) {
	                        httpURLConnection.disconnect();
	                        httpURLConnection = null;
	                    }
	                    return null;
	                }
	            } else if (transformedPath.startsWith("/")) {
	                inputStream = cr.openInputStream(Uri.parse("file://" + transformedPath));
	            }
				if (inputStream == null) {
	                Log.d(TAG, "inputStream = null");
	                if (httpURLConnection != null) {
	                    httpURLConnection.disconnect();
	                    httpURLConnection = null;
	                }
	                return null;
	            }
				BitmapFactory.decodeStream(inputStream, null, options);
				if (inputStream != null) {
	                inputStream.close();
	            }
				if (httpURLConnection != null) {
                    httpURLConnection.disconnect();
                    httpURLConnection = null;
                }
				if (options.mCancel || options.outWidth == -1 || options.outHeight == -1) {
					return null;
				}
				
				// Then read the image content
				options.inJustDecodeBounds = false;			
				if (options.outWidth > mThumbTargetWidth || options.outHeight > mThumbTargetHeight) {
					if (options.outWidth > options.outHeight) {
						options.inSampleSize = Math.round((float)options.outHeight/(float)mThumbTargetHeight);
					} else {
						options.inSampleSize = Math.round((float)options.outWidth/(float)mThumbTargetWidth);
					}
				}
				if (transformedPath.startsWith("http://")) {
				    httpURLConnection = (HttpURLConnection) url.openConnection();
				    httpURLConnection.setConnectTimeout(5000);
	                httpURLConnection.setReadTimeout(5000);
	                httpURLConnection.setRequestMethod("GET");
	                int responseCode;
	                if (200 == (responseCode = httpURLConnection.getResponseCode())) {
	                    inputStream = httpURLConnection.getInputStream();                    
	                } else {
	                    Log.e(TAG, "connect failure! responseCode: " + responseCode);
	                    if (httpURLConnection != null) {
	                        httpURLConnection.disconnect();
	                        httpURLConnection = null;
	                    }
	                    return null;
	                }
                } else if (transformedPath.startsWith("/")) {
                    inputStream = cr.openInputStream(Uri.parse("file://" + transformedPath));
                }
                if (inputStream == null) {
                    Log.d(TAG, "inputStream = null");
                    if (httpURLConnection != null) {
                        httpURLConnection.disconnect();
                        httpURLConnection = null;
                    }
                    return null;
                }				
				bitmapTemp = BitmapFactory.decodeStream(inputStream, null, options);
				if (inputStream != null) {
                    inputStream.close();
                }
				if (httpURLConnection != null) {
                    httpURLConnection.disconnect();
                    httpURLConnection = null;
                }
				
				//bitmap = bitmapTemp;
				bitmap = ThumbnailUtils.extractThumbnail(bitmapTemp, targetWidth, targetHeight, ThumbnailUtils.OPTIONS_RECYCLE_INPUT);
				
			} catch (ConnectTimeoutException  e) {
	            Log.e(TAG, "ConnectTimeoutException", e);
	            return null;
	        } catch (FileNotFoundException e) {
				e.printStackTrace();
				return null;
			} catch (IOException e) {
				e.printStackTrace();
				return null;
			} finally {
	            if (inputStream != null) {
	                try {
	                    inputStream.close();
	                } catch (IOException e) {
	                    e.printStackTrace();
	                }
	            }
	            if (httpURLConnection != null) {
	                httpURLConnection.disconnect();
	                httpURLConnection = null;
	            }
	        }
		}
		
		return bitmap;
	}
	
    /*
     * Compute the sample size as a function of minSideLength
     * and maxNumOfPixels.
     * minSideLength is used to specify that minimal width or height of a
     * bitmap.
     * maxNumOfPixels is used to specify the maximal size in pixels that is
     * tolerable in terms of memory usage.
     *
     * The function returns a sample size based on the constraints.
     * Both size and minSideLength can be passed in as IImage.UNCONSTRAINED,
     * which indicates no care of the corresponding constraint.
     * The functions prefers returning a sample size that
     * generates a smaller bitmap, unless minSideLength = IImage.UNCONSTRAINED.
     *
     * Also, the function rounds up the sample size to a power of 2 or multiple
     * of 8 because BitmapFactory only honors sample size this way.
     * For example, BitmapFactory downsamples an image by 2 even though the
     * request is 3. So we round up the sample size to avoid OOM.
     */
    public static int computeSampleSize(BitmapFactory.Options options, int minSideLength, int maxNumOfPixels) {
        int initialSize = computeInitialSampleSize(options, minSideLength,
                                                   maxNumOfPixels);

        int roundedSize;
        if (initialSize <= 8) {
            roundedSize = 1;
            while (roundedSize < initialSize) {
                roundedSize <<= 1;
            }
        } else {
            roundedSize = (initialSize + 7) / 8 * 8;
        }

        return roundedSize;
    }

    private static int computeInitialSampleSize(BitmapFactory.Options options, int minSideLength, int maxNumOfPixels) {
        double w = options.outWidth;
        double h = options.outHeight;

        int lowerBound = (maxNumOfPixels == UNCONSTRAINED) ? 1 :
                         (int) Math.ceil(Math.sqrt(w * h / maxNumOfPixels));
        int upperBound = (minSideLength == UNCONSTRAINED) ? 128 :
                         (int) Math.min(Math.floor(w / minSideLength),
                                        Math.floor(h / minSideLength));

        if (upperBound < lowerBound) {
            // return the larger one when there is no overlapping zone.
            return lowerBound;
        }

        if ((maxNumOfPixels == UNCONSTRAINED) &&
            (minSideLength == UNCONSTRAINED)) {
            return 1;
        } else if (minSideLength == UNCONSTRAINED) {
            return lowerBound;
        } else {
            return upperBound;
        }
    }

    // Rotates the bitmap by the specified degree.
    // If a new bitmap is created, the original bitmap is recycled.
    public static Bitmap rotate(Bitmap b, int degrees) {
        Bitmap b2 = null;
        if (degrees != 0 && b != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees,
                        (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            try {
                b2 = Bitmap.createBitmap(
                         b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                /*
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
                */
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
            }
        }
        //return b;
        return b2;
    }

}
