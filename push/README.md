The procedure of pushing video to server(RTMP)

Based on some data structs, the functions should be used correctly.

 -------------------------
|  avformat_open_input   | == read the header of file, fetch some info about format
 -------------------------


 --------------------------
|avformat_find_stream_info | == decode one segment of data, fetch related info about stream
 --------------------------

 -------------------------
|      av_dump_format    | == open file and direct to format ctx(AVFormatContext)
 -------------------------

 ------------------------------
|avformat_alloc_output_context2| == alloc the size of output,for about index later
 ------------------------------

 -------------------------
|  avformat_new_stream   | == new a stream for output file
 -------------------------

 -------------------------
| avcodec_parameters_copy| == copy parameters about instream to output stream
 -------------------------

 -----------------------
|    av_dump_format     | == outfile to format ctx
 -----------------------

 -----------------------
|avformat_write_hearder| == to ctx about the header for output
 -----------------------


-----> while for read frame for each frame


 -----------------------
|avformat_write_trailer| == to ctx about the trailer for output
 -----------------------

-----> close all of opened 
