package com.xmap.api;

import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * Created by dell on 2019/8/17.
 */
public abstract class AbstractSourceReader implements SourceReader{

    protected final SourceEntryParser parser;
    protected final Filter filter;
    protected final SourceProcess sourceProcess;

    private ScheduledThreadPoolExecutor scheduledThreadPoolExecutor;
    private SourceReadRunnable sourceReadRunnable;

    protected AbstractSourceReader(SourceEntryParser parser, Filter filter,SourceProcess sourceProcess) {
        this.parser = parser;
        this.sourceProcess = sourceProcess;
        this.filter = filter;
    }

    @Override
    public void start() throws SourceException {

        scheduledThreadPoolExecutor = new ScheduledThreadPoolExecutor(1);

        if (open() == -1) {

            throw new SourceException("Cannot open source reader to read!");

        }
        sourceReadRunnable = new SourceReadRunnable();

        scheduledThreadPoolExecutor.scheduleAtFixedRate(sourceReadRunnable, 500, 500,
                TimeUnit.MILLISECONDS);

    }

    @Override
    public void stop() {

        close();
        scheduledThreadPoolExecutor.remove(sourceReadRunnable);
    }

    private class SourceReadRunnable implements Runnable {

        @Override
        public void run() {


            while (true) {
                XMapEntry mapEntry = read();

                if (mapEntry != null) {
                    if (filter != null && !filter.isAccept(mapEntry)) {
                        readEnd(mapEntry);
                        continue;
                    }

                /*parse it */
                    try {

                        SourceEntry sourceEntry = parser.parse(mapEntry);
                        if(sourceEntry!=null){

                            if(filter.isAccept(sourceEntry)){

                                sourceProcess.process(sourceEntry);
                            }
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                        //log.error(e.getMessage());
                    /*ignore exceptions*/
                    } finally {
                        readEnd(mapEntry);
                    }
                } else {
                    /*na data to read,return thread fun and sleep some times*/
                    break;
                }
            }
        }

    }

}
