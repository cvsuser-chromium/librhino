package com.hybroad.json;

import java.util.ArrayList;

/** must use standard getter and setter for parsing JSON */
public class DMSDeviceListJSON {
    private int count;
    private ArrayList<DMSDeviceJSON> dmsList;

    public DMSDeviceListJSON() {

    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }

    public ArrayList<DMSDeviceJSON> getDmsList() {
        return dmsList;
    }

    public void setDmsList(ArrayList<DMSDeviceJSON> dmsList) {
        this.dmsList = dmsList;
    }

}
