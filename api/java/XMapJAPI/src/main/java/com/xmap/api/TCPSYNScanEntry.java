package com.xmap.api;

import com.xmap.api.utils.MessagePackUtil;
import com.xmap.api.utils.TextUtils;
import org.msgpack.core.MessageUnpacker;

import java.io.IOException;

/**
 * Created by dell on 2019/8/17.
 */
public class TCPSYNScanEntry extends IPScanEntry{

    private int srcPort;
    private int dstPort;
    private long seq;
    private long ack;
    private long window;
    private boolean success;
    private String msg;

    public TCPSYNScanEntry(MessageUnpacker unpacker) throws IOException {

        super(unpacker);
        MessagePackUtil.parseMapHeader(unpacker,true);

        success = MessagePackUtil.parseInt(unpacker)==1;
        msg = MessagePackUtil.parseText(unpacker);
        srcPort = MessagePackUtil.parseInt(unpacker);
        dstPort = MessagePackUtil.parseInt(unpacker);
        seq = MessagePackUtil.parseLong(unpacker);
        ack = MessagePackUtil.parseLong(unpacker);
        window = MessagePackUtil.parseLong(unpacker);
    }

    public String toString(){

        StringBuffer sb = new StringBuffer();
        TextUtils.addText(sb,"success",success?"yes":"no");
        append2String(sb);
        TextUtils.addText(sb,"msg",msg);
        TextUtils.addInt(sb,"srcPort",srcPort);
        TextUtils.addInt(sb,"dstPort",dstPort);
        TextUtils.addLong(sb,"seq",seq);
        TextUtils.addLong(sb,"ack",ack);
        TextUtils.addLong(sb,"window",window);

        return sb.toString();
    }

    public boolean isSuccess() {
        return success;
    }

    public int getSrcPort() {
        return srcPort;
    }

    public void setSrcPort(int srcPort) {
        this.srcPort = srcPort;
    }

    public int getDstPort() {
        return dstPort;
    }

    public void setDstPort(int dstPort) {
        this.dstPort = dstPort;
    }

    public long getSeq() {
        return seq;
    }

    public void setSeq(long seq) {
        this.seq = seq;
    }

    public long getAck() {
        return ack;
    }

    public void setAck(long ack) {
        this.ack = ack;
    }

    public long getWindow() {
        return window;
    }

    public void setWindow(long window) {
        this.window = window;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }
}
