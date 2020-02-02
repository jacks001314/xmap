package com.xmap.api;

import com.xmap.api.utils.IPUtils;
import com.xmap.api.utils.MessagePackUtil;
import com.xmap.api.utils.TextUtils;
import org.msgpack.core.MessageUnpacker;

import java.io.IOException;

public class ICMPTimeScanEntry extends ICMPScanEntry {

    private long sentTimeTS;
    private long sentTimeUS;
    private long dstRawIP;

    public ICMPTimeScanEntry(MessageUnpacker unpacker) throws IOException {
        super(unpacker);

        sentTimeTS = MessagePackUtil.parseLong(unpacker);
        sentTimeUS = MessagePackUtil.parseLong(unpacker);
        dstRawIP = MessagePackUtil.parseLong(unpacker);

    }

    public String toString(){

        StringBuffer sb = new StringBuffer();

        sb.append(super.toString());
        TextUtils.addLong(sb,"sentTimeTS",sentTimeTS);
        TextUtils.addLong(sb,"sentTimeUS",sentTimeUS);
        TextUtils.addText(sb,"dstRawIP", IPUtils.ipv4Str(dstRawIP));

        return sb.toString();
    }


    public long getSentTimeTS() {
        return sentTimeTS;
    }

    public void setSentTimeTS(long sentTimeTS) {
        this.sentTimeTS = sentTimeTS;
    }

    public long getSentTimeUS() {
        return sentTimeUS;
    }

    public void setSentTimeUS(long sentTimeUS) {
        this.sentTimeUS = sentTimeUS;
    }

    public long getDstRawIP() {
        return dstRawIP;
    }

    public void setDstRawIP(long dstRawIP) {
        this.dstRawIP = dstRawIP;
    }
}
