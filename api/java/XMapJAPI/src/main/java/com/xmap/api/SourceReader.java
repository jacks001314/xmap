package com.xmap.api;

/**
 * Created by dell on 2019/8/17.
 */
public interface SourceReader {

    int open();

    XMapEntry read();

    void readEnd(XMapEntry mapEntry);

    void start() throws SourceException;

    void stop();

    void close();
}
