package com.hybroad.json;

public class TrackMetaDataJSON {
    private String dc_title;
    private String mediaType; // Undecided key name
    
    public TrackMetaDataJSON() {
        
    }

    public String getDc_title() {
        return dc_title;
    }

    public void setDc_title(String dc_title) {
        this.dc_title = dc_title;
    }

    public String getMediaType() {
        return mediaType;
    }

    public void setMediaType(String mediaType) {
        this.mediaType = mediaType;
    }
    
}
