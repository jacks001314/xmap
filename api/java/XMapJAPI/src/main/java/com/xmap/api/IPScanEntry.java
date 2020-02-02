package com.xmap.api;

import com.xmap.api.utils.IPUtils;
import com.xmap.api.utils.MessagePackUtil;
import com.xmap.api.utils.TextUtils;
import org.msgpack.core.MessageUnpacker;

import java.io.IOException;

/**
 * Created by dell on 2019/8/17.
 */
public abstract class IPScanEntry implements SourceEntry{

    private int type;
    private long srcIP;
    private long dstIP;
    private long ttl;
    private long id;

    public IPScanEntry(MessageUnpacker unpacker) throws IOException {

       MessagePackUtil.parseMapHeader(unpacker,false);
       this.type = MessagePackUtil.parseInt(unpacker);
       MessagePackUtil.parseMapHeader(unpacker,true);
       this.srcIP = MessagePackUtil.parseLong(unpacker);
       this.dstIP = MessagePackUtil.parseLong(unpacker);
       this.id = MessagePackUtil.parseLong(unpacker);
       this.ttl = MessagePackUtil.parseLong(unpacker);

    }

    public void append2String(StringBuffer sb) {

        TextUtils.addText(sb,"srcIP", IPUtils.ipv4Str(srcIP));
        TextUtils.addText(sb,"dstIP",IPUtils.ipv4Str(dstIP));
        TextUtils.addLong(sb,"ttl",ttl);
        TextUtils.addLong(sb,"id",id);
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public long getSrcIP() {
        return srcIP;
    }

    public void setSrcIP(long srcIP) {
        this.srcIP = srcIP;
    }

    public long getDstIP() {
        return dstIP;
    }

    public void setDstIP(long dstIP) {
        this.dstIP = dstIP;
    }

    public long getTtl() {
        return ttl;
    }

    public void setTtl(long ttl) {
        this.ttl = ttl;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }
}
