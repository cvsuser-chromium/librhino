package com.hybroad.json;

/** must use standard getter and setter for parsing JSON */
public class DMRPlayJSON {
    private String instanceID;
    private String PlayUrl;
    private String mediaType;
    private String speed;
    private TrackMetaDataJSON TrackMetadata;
    // add 20131105 begin
    private double mStartSeconds;

    // add 20131105 end
    public DMRPlayJSON() {

    }

    public String getInstanceID() {
        return instanceID;
    }

    public void setInstanceID(String instanceID) {
        this.instanceID = instanceID;
    }

    public String getPlayUrl() {
        return PlayUrl;
    }

    public void setPlayUrl(String playUrl) {
        PlayUrl = playUrl;
    }

    public String getMediaType() {
        return mediaType;
    }

    public void setMediaType(String mediaType) {
        this.mediaType = mediaType;
    }

    public String getSpeed() {
        return speed;
    }

    public void setSpeed(String speed) {
        this.speed = speed;
    }

    public TrackMetaDataJSON getTrackMetadata() {
        return TrackMetadata;
    }

    public void setTrackMetadata(TrackMetaDataJSON trackMetadata) {
        TrackMetadata = trackMetadata;
    }

    // add 20131105 begin
    public int getStartPoistion() {
        return (int) mStartSeconds;
    }

    public void setStartPoistion(double l) {
        mStartSeconds = l;
    }
    // add 20131105 end
}
