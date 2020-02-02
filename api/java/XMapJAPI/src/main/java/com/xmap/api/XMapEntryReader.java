package com.xmap.api;

import com.xmap.api.utils.TextUtils;

/**
 * Created by dell on 2019/8/17.
 */
public class XMapEntryReader {

    private final XMapEntry mapEntry;
    private final String fname;
    private final String key;
    private final int projID;
    private final int id;

    static {
        System.loadLibrary("XMapJNI");
    };

    public XMapEntryReader(int id,String fname, String key, int projID, long bsize){

        mapEntry = new XMapEntry(bsize);
        this.fname = fname;
        this.key = key;
        this.projID  = projID;

        this.id = id;
    }

    public int open(){

        if(!TextUtils.isEmpty(fname))
            return openMMap(id,fname);

        return openSHM(id,key,projID);
    }

    public native int openMMap(int id,String fname);
    public native int openSHM(int id,String key,int projID);

    public XMapEntry read(){

        int ret = read(id,mapEntry);

        if(ret == 0)
            return mapEntry;

        return null;
    }

    public void close(){

        close(id);

    }

    public native int read(int id,XMapEntry mapEntry);

    public native void close(int id);
}
