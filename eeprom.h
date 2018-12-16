

char loadLastValidChannelList(ChannelList *chl);

/*
	Mode: 0: find a new block to store all Channels
		   1: store 
 */
char storeChannelList(ChannelList *chl);


char storeSingleChannel(ChannelList *chl, char* index, unsigned short ch);

char deleteSingleChannel(ChannelList *chl, char* index);

char getNextChannelIndex(ChannelList *chl, char index, char direct);

char storeFavoriteChannel(ushort ch);

unsigned short loadFavoriteChannel();
