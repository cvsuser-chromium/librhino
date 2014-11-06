package com.hybroad.media;

public class Device {
    private int type; // 0:local 1:dms
    private String name;
    private String rootPath; // for local
    private String deviceID; // for dms

    public void setTypeLocal() {
        this.type = 0;
    }

    public void setTypeDMS() {
        this.type = 1;
    }

    public boolean isLocalDevice() {
        return (this.type == 0);
    }

    public boolean isDMSDevice() {
        return (this.type == 1);
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return this.name;
    }

    public void setRootPath(String path) {
        this.rootPath = path;
    }

    public String getRootPath() {
        return this.rootPath;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public String getDeviceID() {
        return deviceID;
    }

    public void setDeviceID(String deviceID) {
        this.deviceID = deviceID;
    }

}
