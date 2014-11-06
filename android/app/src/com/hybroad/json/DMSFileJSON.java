package com.hybroad.json;

/** must use standard getter and setter for parsing JSON */
public class DMSFileJSON {
    /* for directory, video, music, picture */
    private String objectID;
    private String filename;
    private String classID;
    private String parentID; // no parentID in documentation

    /* for directory */
    private String connectedPort;
    private String StorageUsed;

    /* for video, music, picture */
    private String title; // name of PVR or program
    private String filepath; // it is 'filePath' in documentation
    private int size;
    private int bitrate;
    private String resolution;
    private int duration;
    private int audiochannel;
    private String protocolInfo;
    private int contentSource;
    private String chanKey;
    private String iptvContentID;
    private String iptvUserID;
    private String programRating;
    private int contentType;
    private String bookMarked;
    private String bookmarkPosition;
    private String additionalSubtitle;
    private String compatibleFormat;

    /* for music */
    private String lyricFile;
    private String artist;
    private String album;
    private String year;
    private String genre;
    private String copyright;
    private String composer;
    private String sampleRate;
    private int audioChannel;
    private String url;
    private String coverURL;

    /* for picture */
    private int colorDepth;

    public DMSFileJSON() {
    }

    public String getObjectID() {
        return objectID;
    }

    public void setObjectID(String objectID) {
        this.objectID = objectID;
    }

    public String getFilename() {
        return filename;
    }

    public void setFilename(String filename) {
        this.filename = filename;
    }

    public String getClassID() {
        return classID;
    }

    public void setClassID(String classID) {
        this.classID = classID;
    }

    public String getParentID() {
        return parentID;
    }

    public void setParentID(String parentID) {
        this.parentID = parentID;
    }

    public String getConnectedPort() {
        return connectedPort;
    }

    public void setConnectedPort(String connectedPort) {
        this.connectedPort = connectedPort;
    }

    public String getStorageUsed() {
        return StorageUsed;
    }

    public void setStorageUsed(String storageUsed) {
        StorageUsed = storageUsed;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getFilepath() {
        return filepath;
    }

    public void setFilepath(String filepath) {
        this.filepath = filepath;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public int getBitrate() {
        return bitrate;
    }

    public void setBitrate(int bitrate) {
        this.bitrate = bitrate;
    }

    public String getResolution() {
        return resolution;
    }

    public void setResolution(String resolution) {
        this.resolution = resolution;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public int getAudiochannel() {
        return audiochannel;
    }

    public void setAudiochannel(int audiochannel) {
        this.audiochannel = audiochannel;
    }

    public String getProtocolInfo() {
        return protocolInfo;
    }

    public void setProtocolInfo(String protocolInfo) {
        this.protocolInfo = protocolInfo;
    }

    public int getContentSource() {
        return contentSource;
    }

    public void setContentSource(int contentSource) {
        this.contentSource = contentSource;
    }

    public String getChanKey() {
        return chanKey;
    }

    public void setChanKey(String chanKey) {
        this.chanKey = chanKey;
    }

    public String getIptvContentID() {
        return iptvContentID;
    }

    public void setIptvContentID(String iptvContentID) {
        this.iptvContentID = iptvContentID;
    }

    public String getIptvUserID() {
        return iptvUserID;
    }

    public void setIptvUserID(String iptvUserID) {
        this.iptvUserID = iptvUserID;
    }

    public String getProgramRating() {
        return programRating;
    }

    public void setProgramRating(String programRating) {
        this.programRating = programRating;
    }

    public int getContentType() {
        return contentType;
    }

    public void setContentType(int contentType) {
        this.contentType = contentType;
    }

    public String getBookMarked() {
        return bookMarked;
    }

    public void setBookMarked(String bookMarked) {
        this.bookMarked = bookMarked;
    }

    public String getBookmarkPosition() {
        return bookmarkPosition;
    }

    public void setBookmarkPosition(String bookmarkPosition) {
        this.bookmarkPosition = bookmarkPosition;
    }

    public String getAdditionalSubtitle() {
        return additionalSubtitle;
    }

    public void setAdditionalSubtitle(String additionalSubtitle) {
        this.additionalSubtitle = additionalSubtitle;
    }

    public String getCompatibleFormat() {
        return compatibleFormat;
    }

    public void setCompatibleFormat(String compatibleFormat) {
        this.compatibleFormat = compatibleFormat;
    }

    public String getLyricFile() {
        return lyricFile;
    }

    public void setLyricFile(String lyricFile) {
        this.lyricFile = lyricFile;
    }

    public String getArtist() {
        return artist;
    }

    public void setArtist(String artist) {
        this.artist = artist;
    }

    public String getAlbum() {
        return album;
    }

    public void setAlbum(String album) {
        this.album = album;
    }

    public String getYear() {
        return year;
    }

    public void setYear(String year) {
        this.year = year;
    }

    public String getGenre() {
        return genre;
    }

    public void setGenre(String genre) {
        this.genre = genre;
    }

    public String getCopyright() {
        return copyright;
    }

    public void setCopyright(String copyright) {
        this.copyright = copyright;
    }

    public String getComposer() {
        return composer;
    }

    public void setComposer(String composer) {
        this.composer = composer;
    }

    public String getSampleRate() {
        return sampleRate;
    }

    public void setSampleRate(String sampleRate) {
        this.sampleRate = sampleRate;
    }

    public int getAudioChannel() {
        return audioChannel;
    }

    public void setAudioChannel(int audioChannel) {
        this.audioChannel = audioChannel;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getCoverURL() {
        return coverURL;
    }

    public void setCoverURL(String coverURL) {
        this.coverURL = coverURL;
    }

    public int getColorDepth() {
        return colorDepth;
    }

    public void setColorDepth(int colorDepth) {
        this.colorDepth = colorDepth;
    }

}
