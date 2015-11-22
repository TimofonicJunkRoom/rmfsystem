#include<stdio.h>
#include<stdlib.h>
#include<libxml/xmlmemory.h>
#include<libxml/parser.h>

struct plc_loc
{
	int used;
	int offset;
	int location;
	int length;
};

struct plc_loc real_time_loc[20];
void xml_read(int partid)
{
//	printf("1\n");
	char xml[20];
	int i=0;
	int n;
	xmlChar* value=NULL;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr sensorcur;
	xmlNodePtr detail;
//	printf("1\n");
	xmlKeepBlanksDefault(0);//quite important!
	doc=xmlParseFile("real_time.xml");
	sprintf(xml,"Section%d",partid);
//	printf("1\n");
	while(i<20)
	{
		real_time_loc[i].used=0;
		i++;
	}
//	printf("1\n");
	if(doc==NULL)
	{
		fprintf(stderr,"doc not parse\n");
		return;
	}
	cur=xmlDocGetRootElement(doc);
	if(cur==NULL)
	{
		fprintf(stderr,"empty doc\n");
		return;
	}
	if(xmlStrcmp(cur->name,(const xmlChar*)"root"))
	{
		fprintf(stderr,"wrong type\n");
		xmlFreeDoc(doc);
		return;
	}
//	printf("1\n");
	cur=cur->xmlChildrenNode;
	while(cur!=NULL)
	{	
//		printf("%s\n",cur->name);
		if(!xmlStrcmp(cur->name,(const xmlChar*)xml))
		{
//			printf("1\n");
			sensorcur=cur->xmlChildrenNode;
			i=0;
			while(sensorcur!=NULL)
			{	
//				printf("1\n");
				real_time_loc[i].used=1;
				if(!xmlStrcmp(sensorcur->name,(const xmlChar*)"Sensor"))
				{
					printf("1\n");
					value=xmlGetProp(sensorcur,"SensorOffset");
					real_time_loc[i].offset=atoi(value);
					value=xmlGetProp(sensorcur,"SensorLoc");
					real_time_loc[i].location=atoi(value);
					value=xmlGetProp(sensorcur,"DataLen");
					real_time_loc[i].length=atoi(value);
					i++;
				}
				sensorcur=sensorcur->next;
			}
		}
		cur=cur->next;
	}
}

void main()
{
//	printf("1\n");
	xml_read(1);
	int i=0;
	for(;real_time_loc[i].used!=0;i++)
	{
		printf("%d\n",real_time_loc[i].offset);
	}
}

