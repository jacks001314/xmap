package com.xmap.api;


/**
 * Created by dell on 2018/6/19.
 */
public interface Filter {

    boolean isAccept(XMapEntry mapEntry);

    boolean isAccept(SourceEntry entry);

}
