/^[ ]*\}[ ]*[A-Z]/		{temp = $0;
                   	gsub(";","",$2) ;
                   	print $2 " " FILENAME " " "<[ ]@\\" temp"*>"}

/\#define/              	{temp = $0;        
                        print $1 " " FILENAME " " "<[ ]@\\" temp"*>"}
