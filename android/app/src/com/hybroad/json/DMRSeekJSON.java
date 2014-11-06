package com.hybroad.json;

public class DMRSeekJSON {
    private String instanceID;
    private String seek_mode;
    private String seek_target;
    
    public DMRSeekJSON() {
        
    }

    public String getInstanceID() {
        return instanceID;
    }

    public void setInstanceID(String instanceID) {
        this.instanceID = instanceID;
    }

    public String getSeek_mode() {
        return seek_mode;
    }

    public void setSeek_mode(String seek_mode) {
        this.seek_mode = seek_mode;
    }

    public String getSeek_target() {
        return seek_target;
    }

    public void setSeek_target(String seek_target) {
        this.seek_target = seek_target;
    }
    
}
