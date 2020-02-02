package com.xmap.api;

import com.google.common.io.ByteArrayDataOutput;
import com.google.common.io.ByteStreams;
import com.xmap.api.utils.TextUtils;
import org.msgpack.core.MessagePack;
import org.msgpack.core.MessageUnpacker;

import java.io.DataInput;
import java.nio.ByteBuffer;

/**
 * Created by dell on 2019/8/17.
 */
public class XMapEntry {

    public final static int TCPSYNSCAN= 1;
    public final static int ICMPSCAN=2;
    public final static int ICMPTIMESCAN=3;

    /*64K*/
    public final static int DATA_SIZE_MIN = 65536;

    public int type;
    public int dataSize;

    public ByteBuffer dataBuffer;

    public XMapEntry(long bsize){

        if(bsize<DATA_SIZE_MIN)
            bsize = DATA_SIZE_MIN;

        dataBuffer = ByteBuffer.allocateDirect((int) bsize);

        reset();
    }

    public DataInput getDataInput(){

        byte[] data = new byte[dataSize];

        dataBuffer.get(data);

        return ByteStreams.newDataInput(data);

    }

    public byte[] getData(){

        byte[] data = new byte[dataSize];
        dataBuffer.get(data);

        return data;
    }

    public byte[] getDataWithTypeLen(){

        ByteArrayDataOutput out = ByteStreams.newDataOutput(dataSize+2+8);

        out.writeShort(type);

        out.write(getData());

        return out.toByteArray();
    }

    public MessageUnpacker getMessageUnpacker(){

        byte[] data = new byte[dataSize];

        dataBuffer.get(data);

        return MessagePack.newDefaultUnpacker(data);

    }

    public void reset(){

        dataSize = 0;
        dataBuffer.clear();
    }

    public void setDataSize(int dataSize) {
        this.dataSize = dataSize;
    }

    public int getDataSize() {
        return dataSize;
    }

    public int getType(){

        return type;

    }

    public void setType(int type){

        this.type = type;
    }

    @Override
    public String toString(){

        StringBuffer sb = new StringBuffer();

        TextUtils.addInt(sb,"type",type);
        TextUtils.addInt(sb,"dataSize",dataSize);

        return sb.toString();
    }

}
