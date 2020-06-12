package com.xmap.api.test;

import com.xmap.api.XMapIPIterator;

public class XMapIPIteratorTest {

    public static void main(String[] args){

        if(args.length<4){
            System.out.println("Usage:<wlistPath><blistPath><outPath><port>");
            System.exit(-1);
        }

        String wlistPath = args[0];
        String blistPath = args[1];
        String outPath = args[2];
        int port = Integer.parseInt(args[3]);

        XMapIPIterator xMapIPIterator = new XMapIPIterator();
        System.out.println(xMapIPIterator.iterate(wlistPath,blistPath,outPath,port));
    }

}
