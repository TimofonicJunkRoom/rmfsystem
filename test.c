#include<stdio.h>
#include<stdlib.h>
#include<libxml/xmlmemory.h>
#include<libxml/parser.h>

struct plc_loc
{
	int used;
	int sensorID;
	int location;
	int length;
};

struct plc_loc real_time_loc[20];
void xml_read(int partid)
{
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
		if(!xmlStrcmp(cur->name,(const xmlChar*)"SectionId"))
		{
//			printf("1\n");
			value=xmlGetProp(cur,"PartID");
			i=atoi(value);
			if(i==partid)
			{
//				printf("1\n");
				sensorcur=cur->xmlChildrenNode;
				i=0;
				while(sensorcur!=NULL)
				{
//					printf("1\n");
					if(!xmlStrcmp(sensorcur->name,(const xmlChar*)"Sensors"))
					{
//						printf("1\n");
						detail=sensorcur->xmlChildrenNode;
						while(detail!=NULL)
						{	
//							printf("1\n");
							real_time_loc[i].used=1;
							if(!xmlStrcmp(detail->name,(const xmlChar*)"Sensor"))
							{
								value=xmlGetProp(detail,"SensorID");
								real_time_loc[i].sensorID=atoi(value);
								value=xmlGetProp(detail,"SensorLoc");
								real_time_loc[i].location=atoi(value);
								value=xmlGetProp(detail,"DataLen");
								real_time_loc[i].length=atoi(value);
								i++;
							}
							detail=detail->next;
						}
					}
					sensorcur=sensorcur->next;
				}

			}
		}
		cur=cur->next;
	}
}

void main()
{
	xml_read(9);
	int i=0;
	for(;real_time_loc[i].used!=0;i++)
	{
		printf("%d\n",real_time_loc[i].sensorID);
	}
}

