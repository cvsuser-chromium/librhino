package com.hybroad.json;

import java.util.ArrayList;

/** must use standard getter and setter for parsing JSON */
public class DMSFileListJSON {
    private int count;
    private ArrayList<DMSFileJSON> fileList;

    public DMSFileListJSON() {
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }

    public ArrayList<DMSFileJSON> getFileList() {
        return fileList;
    }

    public void setFileList(ArrayList<DMSFileJSON> fileList) {
        this.fileList = fileList;
    }

}
