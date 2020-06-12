package com.xmap.api;

public class XMapIPIterator {

    static {
        System.loadLibrary("XMapJNI");
    };


    public native int iterate(String wlistPath,String blistPath,String outPath,int port);
    public native int iterate(String wlistPath,String blistPath,String outPath,String ports);
}
