package com.xmap.api;

/**
 * Created by dell on 2019/8/17.
 */
public class XMapSourceReader extends AbstractSourceReader {

    private XMapEntryReader mapEntryReader;

    public XMapSourceReader(Filter filter,SourceProcess sourceProcess,int id,String mmapFileName) {

        super(new XMapSourceEntryParser(), filter,sourceProcess);

        this.mapEntryReader = new XMapEntryReader(id,mmapFileName,null,0,65536);

    }

    @Override
    public int open() {

        return mapEntryReader.open();
    }

    @Override
    public XMapEntry read() {

        return mapEntryReader.read();
    }

    @Override
    public void readEnd(XMapEntry mapEntry) {

        mapEntry.reset();
    }

    @Override
    public void close() {

        mapEntryReader.close();
    }

}
