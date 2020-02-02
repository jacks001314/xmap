package com.xmap.api;

import java.io.IOException;

/**
 * Created by dell on 2019/8/17.
 */
public class XMapSourceEntryParser implements SourceEntryParser {

    @Override
    public SourceEntry parse(XMapEntry mapEntry) throws IOException {
        SourceEntry entry = null;
        int type = mapEntry.getType();

        switch (type) {

            case XMapEntry.TCPSYNSCAN:
                entry = new TCPSYNScanEntry(mapEntry.getMessageUnpacker());
                break;

            case XMapEntry.ICMPSCAN:
                entry = new ICMPScanEntry(mapEntry.getMessageUnpacker());
                break;

            case XMapEntry.ICMPTIMESCAN:
                entry = new ICMPTimeScanEntry(mapEntry.getMessageUnpacker());
                break;

            default:
                entry = null;
                break;
        }

        return entry;
    }

}
