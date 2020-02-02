package com.xmap.api;

import com.xmap.api.utils.MessagePackUtil;
import com.xmap.api.utils.TextUtils;
import org.msgpack.core.MessageUnpacker;

import java.io.IOException;

public class ICMPScanEntry extends IPScanEntry {

    private boolean success;
    private String msg;
    private int type;
    private int code;
    private long id;
    private long seq;

    public ICMPScanEntry(MessageUnpacker unpacker) throws IOException {
        super(unpacker);

        MessagePackUtil.parseMapHeader(unpacker,true);
        success = MessagePackUtil.parseInt(unpacker)==1;
        msg = MessagePackUtil.parseText(unpacker);
        type = MessagePackUtil.parseInt(unpacker);
        code = MessagePackUtil.parseInt(unpacker);
        id = MessagePackUtil.parseLong(unpacker);
        seq = MessagePackUtil.parseLong(unpacker);

    }

    public String toString(){

        StringBuffer sb = new StringBuffer();
        TextUtils.addText(sb,"success",success?"yes":"no");
        append2String(sb);
        TextUtils.addText(sb,"msg",msg);
        TextUtils.addInt(sb,"type",type);
        TextUtils.addInt(sb,"code",code);
        TextUtils.addLong(sb,"id",id);
        TextUtils.addLong(sb,"seq",seq);

        return sb.toString();
    }

    public boolean isSuccess() {
        return success;
    }

    public void setSuccess(boolean sucess) {
        this.success = sucess;
    }

    public String getMsg() {
        return msg;
    }

    public void setMsg(String msg) {
        this.msg = msg;
    }

    @Override
    public int getType() {
        return type;
    }

    @Override
    public void setType(int type) {
        this.type = type;
    }

    public int getCode() {
        return code;
    }

    public void setCode(int code) {
        this.code = code;
    }

    @Override
    public long getId() {
        return id;
    }

    @Override
    public void setId(long id) {
        this.id = id;
    }

    public long getSeq() {
        return seq;
    }

    public void setSeq(long seq) {
        this.seq = seq;
    }
}
