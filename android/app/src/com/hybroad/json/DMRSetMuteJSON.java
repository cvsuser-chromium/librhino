package com.hybroad.json;

public class DMRSetMuteJSON {
    private String InstanceID;
    private String Channel;
    private String Mute;
    
    public DMRSetMuteJSON() {
        
    }
    
    public String getInstanceID() {
        return InstanceID;
    }
    
    public void setInstanceID(String instanceID) {
        InstanceID = instanceID;
    }
    
    public String getChannel() {
        return Channel;
    }
    
    public void setChannel(String channel) {
        Channel = channel;
    }
    
    public String getMute() {
        return Mute;
    }
    
    public void setMute(String mute) {
        Mute = mute;
    }
    
}
