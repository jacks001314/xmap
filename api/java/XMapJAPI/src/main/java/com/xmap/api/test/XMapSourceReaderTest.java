package com.xmap.api.test;

import com.xmap.api.*;
import com.xmap.api.utils.IPUtils;
import com.xmap.api.utils.KeepAliveThread;

/**
 * Created by dell on 2019/8/17.
 */
public class XMapSourceReaderTest {

    public static void main(String[] args) throws SourceException {

        if(args.length<1)
        {
            System.out.println("Usage:<mmapfilename>");
            System.exit(-1);
        }

        SourceReader sourceReader = new XMapSourceReader(new Filter() {
            @Override
            public boolean isAccept(XMapEntry mapEntry) {
                return true;
            }

            @Override
            public boolean isAccept(SourceEntry entry) {
                return true;
            }
        }, new SourceProcess() {
            @Override
            public void process(SourceEntry sourceEntry) {

                if(sourceEntry instanceof TCPSYNScanEntry){

                    TCPSYNScanEntry scanEntry = (TCPSYNScanEntry)sourceEntry;
                    if(scanEntry.isSuccess()){

                        System.out.println(IPUtils.ipv4Str(scanEntry.getSrcIP()));
                    }
                }else
                    System.out.println(sourceEntry);
            }
        },
                0, args[0]);


        sourceReader.start();

          /*keepalive main thread*/
        Runtime.getRuntime().addShutdownHook(new Thread("SourceReaderMain-shutdown-hook") {
            @Override
            public void run() {
                System.err.println("EventEngineMain exit -------------------------------------------------------------------------------kao");;
            }
        });

        KeepAliveThread kpt = new KeepAliveThread("TestSourceReader");

        kpt.start();

    }
}
