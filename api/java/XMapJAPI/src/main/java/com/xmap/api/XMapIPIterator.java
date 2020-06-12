package com.xmap.api;

public class XMapIPIterator {

    static {
        System.loadLibrary("XMapJNI");
    };


    public native int iterate(String wlistPath,String blistPath,String outPath,int port);
    public native int iterate(String wlistPath,String blistPath,String outPath,String ports);
    
    public native int prepareIterate(String wlistPath,String blistPath);
    public native long getNextIP();
    public native void closeIterate();
}
