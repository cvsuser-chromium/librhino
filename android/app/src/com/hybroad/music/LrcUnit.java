package com.hybroad.music;

public class LrcUnit {
    private int beginTime = 0;
    private int lineTime = 0;
    private String lrcBody = null;

    public int getBeginTime() {
        return beginTime;
    }

    public void setBeginTime(int beginTime) {
        this.beginTime = beginTime;
    }

    public int getLineTime() {
        return lineTime;
    }

    public void setLineTime(int lineTime) {
        this.lineTime = lineTime;
    }

    public String getLrcBody() {
        return lrcBody;
    }

    public void setLrcBody(String lrcBody) {
        this.lrcBody = lrcBody;
    }
}
