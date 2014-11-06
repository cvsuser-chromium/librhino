package com.hybroad.dlna;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;

import com.hybroad.dlna.DMRDeviceClient.OnDMRRequestEvent;
import com.hybroad.json.DMRPlayJSON;
import com.hybroad.json.DMRSeekJSON;
import com.hybroad.json.DMRSetMuteJSON;
import com.hybroad.json.DMRSetVolumeJSON;
import com.hybroad.json.JSONUtils;
import com.hybroad.json.TrackMetaDataJSON;
import com.hybroad.util.Converter;
import com.hybroad.util.NetworkUtils;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.SoundUtils;

public class DMRService extends Service {
    private final String TAG = "---DMRService---";

    private static DMRDeviceClient mDMRDeviceClient = null;
    private boolean isCreateDMRClientThreadLocked = false;
    private Thread mCreateDMRClientThread = null;
    private long mCreateDMRClientThreadCount = 0;
    private boolean isServerDied = false;
    private boolean isReallyStop = false;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand");

        if (NetworkUtils.isNetworkConnected(DMRService.this)
                && !PublicConstants.DLNACommon.DLNA_BIN_SERVICE_START_VALUE
                        .equals(System
                                .getProperty(PublicConstants.DLNACommon.DLNA_BIN_SERVICE_START_KEY))) {
            Log.d(TAG, "start dlna server...");
            System.getProperty(
                    PublicConstants.DLNACommon.DLNA_BIN_SERVICE_START_KEY,
                    PublicConstants.DLNACommon.DLNA_BIN_SERVICE_START_VALUE);
        }

        if (NetworkUtils.isNetworkConnected(DMRService.this)
                && null == mDMRDeviceClient) {
            startCreateDMRClientThread();
        }

        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return null;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind");
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        Intent intent = new Intent();
        intent.setClass(this, DMRService.class);
        this.startService(intent);
    }

    private void setDMRRequestEvent(final Context context) {
        if (null == mDMRDeviceClient)
            return;

        Log.d(TAG, "--DMR----mDMRDeviceClient.setDMRRequestEvent");
        mDMRDeviceClient.setDMRRequestEvent(new OnDMRRequestEvent() {
            @Override
            public String onEvent(DMRDeviceClient mp, int what, int eventID,
                    int arg2, String jsonString) {
                // Log.d(TAG, "--DMR----OnDMRRequestEvent-----mp: " + mp);
                Log.d(TAG, "--DMR----OnDMRRequestEvent-----what: " + what);
                Log.d(TAG, "--DMR----OnDMRRequestEvent-----eventID: " + eventID);
                // Log.d(TAG, "--DMR----OnDMRRequestEvent-----arg2: " + arg2);
                Log.d(TAG, "--DMR----OnDMRRequestEvent-----jsonString: "
                        + jsonString);

                Intent intent = new Intent(
                        PublicConstants.DMRIntent.ACTION_DMR_EVENT);
                String returnJson = "{ \"returncode\": \"-911\" }";

                if (DMRDeviceClient.EVENT_DLNA_ERROR == what) {
                    switch (eventID) {
                    case DMRDeviceClient.DLNA_ERROR_SERVER_DIED:
                        Log.d(TAG, "DLNA ERROR: server died");
                        isServerDied = true;
                        mDMRDeviceClient = null;
                        DMRSharingInformation.resetDMRShareData();
                        Intent serverDiedIntent = new Intent(
                                PublicConstants.DMRIntent.ACTION_DLNA_SERVER_DIED);
                        sendBroadcast(serverDiedIntent);
                        // createDMRClientAfterSomeTime(5000);
                        startCreateDMRClientThread();
                        break;

                    case DMRDeviceClient.DLNA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK:
                        Log.d(TAG,
                                "DLNA ERROR: NOT_VALID_FOR_PROGRESSIVE_PLAYBACK");
                        break;

                    case DMRDeviceClient.DLNA_ERROR_UNKNOWN:
                        Log.d(TAG, "DLNA ERROR: unknown error");
                        break;

                    default:
                        break;
                    }
                } else {
                    switch (eventID) {
                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETMUTE:
                        Log.d(TAG, "to set mute ");
                        if (TextUtils.isEmpty(jsonString)) {
                            Log.d(TAG, "jsonString is empty");
                            break;
                        }
                        DMRSetMuteJSON setMuteJSON = new DMRSetMuteJSON();
                        setMuteJSON = (DMRSetMuteJSON) JSONUtils
                                .loadObjectFromJSON(jsonString, setMuteJSON);
                        if (null == setMuteJSON) {
                            Log.d(TAG, "setMuteJSON is null");
                            break;
                        }
                        String setMuteString = setMuteJSON.getMute();
                        if (TextUtils.isEmpty(setMuteString)) {
                            Log.d(TAG, "setMuteString is empty");
                            break;
                        }
                        if (PublicConstants.DLNACommon.SET_MUTE_CONFIRM
                                .equals(setMuteString)) {
                            SoundUtils.setMute(context, true);
                        } else if (PublicConstants.DLNACommon.SET_MUTE_CANCEL
                                .equals(setMuteString)) {
                            SoundUtils.setMute(context, false);
                        }
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME:
                        Log.d(TAG, "to set volume");
                        if (TextUtils.isEmpty(jsonString)) {
                            Log.d(TAG, "jsonString is empty");
                            break;
                        }
                        DMRSetVolumeJSON setVolumeJSON = new DMRSetVolumeJSON();
                        setVolumeJSON = (DMRSetVolumeJSON) JSONUtils
                                .loadObjectFromJSON(jsonString, setVolumeJSON);
                        if (null == setVolumeJSON) {
                            Log.d(TAG, "setVolumeJSON is null");
                            break;
                        }
                        String channel = setVolumeJSON.getChannel();
                        if (DLNAConstant.Channel.STEREO.equals(channel)) {
                            SoundUtils.setVolumePercent(context,
                                    setVolumeJSON.getVolume());
                        } else if (DLNAConstant.Channel.LEFT_FRONT
                                .equals(channel)) {
                            DMRSharingInformation.sLeftVolume = setVolumeJSON
                                    .getVolume() * 0.01f;
                            intent.putExtra(
                                    PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                    PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME);
                            context.sendBroadcast(intent);
                        } else if (DLNAConstant.Channel.RIGHT_FRONT
                                .equals(channel)) {
                            DMRSharingInformation.sRightVolume = setVolumeJSON
                                    .getVolume() * 0.01f;
                            intent.putExtra(
                                    PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                    PublicConstants.DMREvent.DLNA_EVENT_DMR_SETVOLUME);
                            context.sendBroadcast(intent);
                        }
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETMUTE:
                        int mute = SoundUtils.isMute(context) ? 1 : 0;
                        returnJson = "{ \"Mute\": \"" + mute + "\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETVOLUME:
                        int volume = SoundUtils.getVolume(context);
                        int max = SoundUtils.getMaxVolume(context);
                        int percent = (int) ((float) volume / max * 100);
                        returnJson = "{ \"Volume\": \"" + percent + "\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY:
                        isReallyStop = false;
                        if (TextUtils.isEmpty(jsonString)) {
                            Log.d(TAG, "jsonString is empty");
                            break;
                        }
                        DMRPlayJSON playJson = new DMRPlayJSON();
                        playJson = (DMRPlayJSON) JSONUtils.loadObjectFromJSON(
                                jsonString, playJson);
                        if (null == playJson) {
                            Log.d(TAG, "playJson is null");
                            break;
                        }
                        String tempUrl = playJson.getPlayUrl();
                        Log.d(TAG, "request play url: " + tempUrl);
                        if (TextUtils.isEmpty(tempUrl)) {
                            Log.d(TAG, "playJson.getPlayUrl is empty");
                            break;
                        }
                        if (tempUrl
                                .equals(DMRSharingInformation.sCurrentPlayUrl)) { // play
                                                                                  // from
                                                                                  // pause
                            Log.d(TAG, "play from pause---->"
                                    + DMRSharingInformation.sCurrentPlayUrl);
                            intent.putExtra(
                                    PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                    PublicConstants.DMREvent.DLNA_EVENT_DMR_PLAY);
                            context.sendBroadcast(intent);
                        } else { // start a new player
                            DMRSharingInformation.sCurrentPlayUrl = tempUrl;
                            String mediaType = null;
                            String originalTitle = "";
                            TrackMetaDataJSON TrackMetadata = playJson
                                    .getTrackMetadata();
                            if (null != TrackMetadata) {
                                mediaType = TrackMetadata.getMediaType();
                                originalTitle = TrackMetadata.getDc_title();
                                Log.d(TAG, "originalTitle: " + originalTitle);
                            }
                            if (TextUtils.isEmpty(mediaType)) {
                                mediaType = playJson.getMediaType();
                            }

                            if (PublicConstants.DLNACommon.MEDIA_TYPE_VIDEO
                                    .equals(mediaType)) {
                                Log.d(TAG, "startVideoPlayer---->"
                                        + DMRSharingInformation.sCurrentPlayUrl);
                                startVideoPlayer(context,
                                        DMRSharingInformation.sCurrentPlayUrl,
                                        0, originalTitle);
                            } else if (PublicConstants.DLNACommon.MEDIA_TYPE_AUDIO
                                    .equals(mediaType)) {
                                Log.d(TAG, "startMusicPlayer---->"
                                        + DMRSharingInformation.sCurrentPlayUrl);
                                startMusicPlayer(context,
                                        DMRSharingInformation.sCurrentPlayUrl,
                                        0, originalTitle);
                            } else if (PublicConstants.DLNACommon.MEDIA_TYPE_PICTURE
                                    .equals(mediaType)) {
                                Log.d(TAG, "startImagePlayer---->"
                                        + DMRSharingInformation.sCurrentPlayUrl);
                                startImagePlayer(context,
                                        DMRSharingInformation.sCurrentPlayUrl,
                                        originalTitle);
                            }
                        }
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE:
                        Log.d(TAG, "to pause ");
                        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                PublicConstants.DMREvent.DLNA_EVENT_DMR_PAUSE);
                        context.sendBroadcast(intent);
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME: // no
                                                                         // use
                        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                PublicConstants.DMREvent.DLNA_EVENT_DMR_RESUME);
                        context.sendBroadcast(intent);
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK:
                        Log.d(TAG, "to seek ");
                        if (TextUtils.isEmpty(jsonString)) {
                            Log.d(TAG, "jsonString is empty");
                            break;
                        }
                        DMRSeekJSON seekJSON = new DMRSeekJSON();
                        seekJSON = (DMRSeekJSON) JSONUtils.loadObjectFromJSON(
                                jsonString, seekJSON);
                        if (null == seekJSON) {
                            Log.d(TAG, "seekJSON is null");
                            break;
                        }
                        if (TextUtils.isEmpty(seekJSON.getSeek_target())) {
                            Log.d(TAG, "seekJSON.getSeek_target is empty");
                            break;
                        }
                        int seek = (int) Converter
                                .convertHoursMinutesSecondsToMsec(seekJSON
                                        .getSeek_target());
                        Log.d(TAG, "to seek: " + seek);
                        DMRSharingInformation.sInitSeekPosition = seek; // for
                                                                        // first
                                                                        // seek
                        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                PublicConstants.DMREvent.DLNA_EVENT_DMR_SEEK);
                        intent.putExtra(
                                PublicConstants.DMRIntent.KEY_SEEK_VALUE, seek);
                        context.sendBroadcast(intent);
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP:
                        Log.d(TAG, "to stop ");
                        isReallyStop = true;
                        intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                                PublicConstants.DMREvent.DLNA_EVENT_DMR_STOP);
                        context.sendBroadcast(intent);
                        confirmReallyStop(context, intent);
                        returnJson = "{ \"returncode\": \"0\" }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETPOSITIONINFO:
                        String url = "";
                        if (!TextUtils
                                .isEmpty(DMRSharingInformation.sCurrentPlayUrl))
                            url = DMRSharingInformation.sCurrentPlayUrl;

                        // Don't send TrackMetaData to server, or server will
                        // not send
                        // TrackMetaData to DMC.
                        returnJson = "{ " + "\"Position\": \""
                                + DMRSharingInformation.sCurrentPlayPostion
                                + "\", " + "\"Duration\": \""
                                + DMRSharingInformation.sDuration + "\", "
                                + "\"returncode\": \"0\", "
                                + "\"Track\": \"0\", "
                                + "\"RelCount\": \"0\", "
                                + "\"AbsCount\": \"0\", " + "\"TrackURI\": \""
                                + url + "\"" + " }";
                        // +
                        // "\"TrackMetaData\": \"&lt;DIDL-Lite xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot; xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot;&gt;&lt;item id=&quot;0&quot; parentID=&quot;-1&quot; restricted=&quot;1&quot;&gt;&lt;dc:title&gt;ice_age_drift_of_continents&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.videoItem&lt;/upnp:class&gt;&lt;res protocolInfo=&quot;http-get:*:video/mp4:*&quot;&gt;"
                        // + url +
                        // "&lt;/res&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;\", "

                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETTRANSPORTINFO:
                        returnJson = "{ " + "\"CurrentTransportState\": \""
                                + DMRSharingInformation.sTransportState
                                + "\", " + "\"CurrentTransportStatus\": \""
                                + DMRSharingInformation.sTransportStatus
                                + "\", " + "\"CurrentSpeed\": \"1\"" + " }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETMEDIAINFO:
                        url = "";
                        if (!TextUtils
                                .isEmpty(DMRSharingInformation.sCurrentPlayUrl))
                            url = DMRSharingInformation.sCurrentPlayUrl;

                        returnJson = "{ " + "\"CurrentURI\": \"" + url + "\", "
                                + "\"MediaDuration\": \""
                                + DMRSharingInformation.sDuration + "\", "
                                + "\"NrTracks\": \"0\", "
                                + "\"WriteStatus\": \"NOT_IMPLEMENTED\", "
                                + "\"CurrentURIMetaData\": \"\", "
                                + "\"NextURI\": \"\", "
                                + "\"NextURIMetaData\": \"\", "
                                + "\"PlayMedium\": \"\", "
                                + "\"RecordMedium\": \"\"" + " }";
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM:
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_GETPRODUCTINFO:
                        break;

                    case PublicConstants.DMREvent.DLNA_EVENT_DMR_ORDER:
                        break;

                    default:
                        break;
                    }
                }

                Log.d(TAG, "------returnJson: " + returnJson);
                return returnJson;
            }
        });
    }

    private void startImagePlayer(Context context, String filePath,
            String originalTitle) {
        Intent intent3 = new Intent();
        intent3.setClassName("com.hybroad.media",
                "com.hybroad.image.ImagePlayer");
        intent3.putExtra(PublicConstants.DMRIntent.KEY_URL, filePath);
        intent3.putExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE,
                originalTitle);
        intent3.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent3);
    }

    private void startMusicPlayer(Context context, String filePath,
            int seekValue, String originalTitle) {
        Intent intent2 = new Intent();
        intent2.setClassName("com.hybroad.media",
                "com.hybroad.music.MusicPlayer");
        intent2.putExtra(PublicConstants.DMRIntent.KEY_URL, filePath);
        intent2.putExtra(PublicConstants.DMRIntent.KEY_SEEK_VALUE, seekValue);
        intent2.putExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE,
                originalTitle);
        intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent2);
    }

    private void startVideoPlayer(Context context, String filePath,
            int seekValue, String originalTitle) {
        Intent intent = new Intent();
        intent.setClassName("com.hybroad.media",
                "com.hybroad.video.VideoPlayer");
        intent.putExtra(PublicConstants.DMRIntent.KEY_URL, filePath);
        intent.putExtra(PublicConstants.DMRIntent.KEY_SEEK_VALUE, seekValue);
        intent.putExtra(PublicConstants.DMRIntent.KEY_ORIGINAL_TITLE,
                originalTitle);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    private void createDMRClientAfterSomeTime(final long delay) {
        Timer timer = new Timer();
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                if ("running"
                        .equals(System
                                .getProperty(PublicConstants.DLNACommon.DLNA_BIN_SERVICE_STATE_PROPERTY))) {
                    if (null == mDMRDeviceClient) {
                        Log.d(TAG, "create DMRClient After " + delay + "ms");
                        LibraryLoader.ensureInitialized();
                        mDMRDeviceClient = DMRDeviceClient
                                .create(DMRService.this);
                        setDMRRequestEvent(DMRService.this);
                        Intent serverStartIntent = new Intent(
                                PublicConstants.DMRIntent.ACTION_DLNA_SERVER_STARTED);
                        sendBroadcast(serverStartIntent);
                    }
                } else {
                    createDMRClientAfterSomeTime(delay);
                }
                this.cancel();
            }
        };
        timer.schedule(task, delay);
    }

    private void confirmReallyStop(final Context context, final Intent intent) {
        Timer timer = new Timer();
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                if (isReallyStop) {
                    intent.putExtra(PublicConstants.DMRIntent.KEY_PLAY_CMD,
                            PublicConstants.DMREvent.DLNA_EVENT_DMR_EXIT);
                    context.sendBroadcast(intent);
                }
                this.cancel();
            }
        };
        timer.schedule(task, 500);
    }

    private void restartService() {
        Intent intent = new Intent();
        intent.setClass(this, DMRService.class);
        this.stopService(intent);
        this.startService(intent);
    }

    private void startCreateDMRClientThread() {
        stopCreateDMRClientThread();
        mCreateDMRClientThread = new Thread(new CreateDMRClientRunnable(
                mCreateDMRClientThreadCount));
        mCreateDMRClientThread.start();
    }

    private void stopCreateDMRClientThread() {
        mCreateDMRClientThreadCount++;
        mCreateDMRClientThread = null;
    }

    class CreateDMRClientRunnable implements Runnable {
        private long mThreadID = 0;

        public CreateDMRClientRunnable(long count) {
            CreateDMRClientRunnable.this.mThreadID = count;
        }

        @Override
        public void run() {
            if (true)
                return;
            Log.d(TAG, "==>mCreateDMRClientThread----start: mCount="
                    + mThreadID + ", count=" + mCreateDMRClientThreadCount);
            if (this.mThreadID != mCreateDMRClientThreadCount) {
                Log.d(TAG,
                        "==>mCreateDMRClientThread----early termination: mCount="
                                + mThreadID + ", count="
                                + mCreateDMRClientThreadCount);
                return;
            }

            while (isCreateDMRClientThreadLocked) {
                try {
                    Log.d(TAG, "mCreateDMRClientThread:" + this.mThreadID
                            + " is locked, waitting......");
                    Thread.currentThread();
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (this.mThreadID != mCreateDMRClientThreadCount) {
                    Log.d(TAG,
                            "==>mCreateDMRClientThread----early termination: mCount="
                                    + mThreadID + ", count="
                                    + mCreateDMRClientThreadCount);
                    return;
                }
            }
            isCreateDMRClientThreadLocked = true;

            // work area
            {
                while (true) {
                    if ("running"
                            .equals(System
                                    .getProperty(PublicConstants.DLNACommon.DLNA_BIN_SERVICE_STATE_PROPERTY))) {
                        if (null == mDMRDeviceClient) {
                            LibraryLoader.ensureInitialized();
                            mDMRDeviceClient = DMRDeviceClient
                                    .create(DMRService.this);
                            setDMRRequestEvent(DMRService.this);
                        }
                        if (isServerDied) {
                            isServerDied = false;
                            Intent serverStartIntent = new Intent(
                                    PublicConstants.DMRIntent.ACTION_DLNA_SERVER_STARTED);
                            sendBroadcast(serverStartIntent);
                        }
                        break;
                    } else {
                        Thread.currentThread();
                        try {
                            Thread.sleep(3000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                    if (this.mThreadID != mCreateDMRClientThreadCount) {
                        Log.d(TAG,
                                "==>mCreateDMRClientThread----early termination: mCount="
                                        + mThreadID + ", count="
                                        + mCreateDMRClientThreadCount);
                        isCreateDMRClientThreadLocked = false;
                        return;
                    }
                }
            } // end work

            Log.d(TAG, "==>mCreateDMRClientThread----end: mCount=" + mThreadID
                    + ", count=" + mCreateDMRClientThreadCount);
            isCreateDMRClientThreadLocked = false;
            return;
        } // end run()
    }
}
