#!/bin/bash
# speak the last highlighted text
qdbus org.kde.klipper /klipper getClipboardContents |			# get klipper Contents
sed ':a;N;$!ba;s/\n/ /g' |						# replace all \n with \s
sed 's/\(http\|www\)[^\ ]*/ URL /g' | 					# filter out web adresses
sed 's/[0-9]\{6,\}/ "large number" /g'  |				# remove long numbers
sed 's/permalink\([^\ ]*\ \)\{0,6\}goldreply/\./g' |			# reddit_filter
sed 's/\[\–\]\([^\ ]*\ \)\{0,9\}ago/"next\."/g' |			# reddit_filter
espeak --stdin -a 70 -p 56 -s 230 -g 1					# speak it:  volume, pitch , speed, pause(gap)time



#                                                                          s=find and replace          g= for all matches in the string
#   23 [–]maleia 49 points an hour ago 23  => 23 23
#                                                                              sed 'option/   pattern / replace with (empty=nothing) / option' 

#  \(aaa\|bbb\)  means either aaa OR bbb        \(parent\|\) means parent or nothing

#                                                                                    [^\ ]* matches all charakters untill it finds a space       .* =all     [^h]*=all exept h




# [0-9] means a dezimal  \{3,5\}  3-5 dezimals in a row  and   \{3,\} means 3-inf    
# [0-9, \. , -] numbers with dots and minuses

# sed 's/[0-9]\{4,8\}/ large number /g'                    1234657 123465 12346
# sed 's/[0-9,\.]\{6,\}/ "large number" /g'

#--stdin because bug else see http://sourceforge.net/p/espeak/bugs/124/

#  qdbus org.kde.klipper /klipper getClipboardContents |   espeak --stdin -a 70 -p 56 -s 230 -g 1

# 23 [–]maleia 49 points an hour ago 23
 #     permalinksaveparentreportgive goldreply
 
 # sed 's/permalinksave\(parent\|\)reportgive goldreply/\./g'