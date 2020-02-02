package com.xmap.api;

import java.io.IOException;

/**
 * Created by dell on 2019/8/17.
 */
public interface SourceEntryParser {
        SourceEntry parse(XMapEntry mapEntry) throws IOException;

}
