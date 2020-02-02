package com.xmap.api.utils;


import com.xmap.api.SourceEntry;
import com.xmap.api.TCPSYNScanEntry;

/**
 * Created by dell on 2018/6/27.
 */
public final class SourceEntryUtil {

    private SourceEntryUtil() {
    }

    public static boolean isTCPSynScan(SourceEntry sourceEntry) {

        return sourceEntry instanceof TCPSYNScanEntry;
    }

    public static TCPSYNScanEntry toTCPSynScan(SourceEntry sourceEntry) {

        return (TCPSYNScanEntry) sourceEntry;
    }

}
