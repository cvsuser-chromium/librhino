package com.hybroad.json;

public class DMRSetVolumeJSON {
    private String InstanceID;
    private String Channel;
    private int Volume;
    
    public DMRSetVolumeJSON() {
        
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
    
    public int getVolume() {
        return Volume;
    }
    
    public void setVolume(int volume) {
        Volume = volume;
    }
    
}
