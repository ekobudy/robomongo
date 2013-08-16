/*
    wdb - weather and water data storage

    Copyright (C) 2007 met.no

    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: wdb@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, USA
*/

#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
//#include <trimstr.h>
#include <sstream>
#include "ptimeutil.h"

using namespace std;

namespace {
	int getInt( const string &timebuf, string::size_type &index, int numberOfChar );
}


std::string miutil::rfc1123date( const boost::posix_time::ptime &pt )
{
   
   char buf[64];
   const char *day=0;
   const char *mon=0;
   unsigned short year;
   
   if( pt.is_special() )
      return std::string();
   
   boost::gregorian::date d( pt.date() );
   boost::posix_time::time_duration t( pt.time_of_day() );
      
   switch( d.day_of_week() ) {
      using namespace boost::date_time;
      case Sunday:    day="Sun"; break;
      case Monday:    day="Mon"; break;
      case Tuesday:   day="Tue"; break;
      case Wednesday: day="Wed"; break;
      case Thursday:  day="Thu"; break;
      case Friday:    day="Fri"; break;
      case Saturday:  day="Sat"; break;
   }
   
   switch( d.month() ){
      using namespace boost::date_time;
      case Jan: mon="Jan"; break;
      case Feb: mon="Feb"; break;
      case Mar: mon="Mar"; break;
      case Apr: mon="Apr"; break;
      case May: mon="May"; break;
      case Jun: mon="Jun"; break;
      case Jul: mon="Jul"; break;
      case Aug: mon="Aug"; break;
      case Sep: mon="Sep"; break;
      case Oct: mon="Oct"; break;
      case Nov: mon="Nov"; break;
      case Dec: mon="Dec"; break;
   }
   
   if( !day || !mon )
	   return std::string();

   year = d.year();
   
   sprintf( buf, "%s, %02d %s %04d %02d:%02d:%02d GMT", 
            day, d.day().as_number(), mon, year,
            t.hours(), t.minutes(), t.seconds() );
   
   return std::string(buf);
}



boost::posix_time::ptime miutil::rfc1123date( const std::string &rfc1123 )
{
   return rfc1123date( rfc1123.c_str() );
}

boost::posix_time::ptime miutil::rfc1123date( const char *rfc1123 )
{
   char gmt[4];
   char sday[4];
   char smon[4];
   int  day, mon, year, h, m, s;
   int  weekDay;
      
   if( !rfc1123 )
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
   
   if ( sscanf( rfc1123,"%3s, %02d %3s %04d %02d:%02d:%02d %3s", sday, &day, smon, &year,
                &h, &m, &s, gmt) !=8 ) 
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
   
   if( strcmp(gmt, "GMT")!=0 )
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
      
   if ( strcmp(smon, "Jan")==0 )
      mon=1;
   else if ( strcmp(smon, "Feb")==0 )
      mon=2;
   else if ( strcmp(smon, "Mar")==0 )
      mon=3;
   else if ( strcmp(smon, "Apr")==0 )
      mon=4;
   else if ( strcmp(smon, "May")==0 )
      mon=5;
   else if ( strcmp(smon, "Jun")==0 )
      mon=6;
   else if ( strcmp(smon, "Jul")==0 )
      mon=7;
   else if ( strcmp(smon, "Aug")==0 )
      mon=8;
   else if ( strcmp(smon, "Sep")==0 )
      mon=9;
   else if ( strcmp(smon, "Oct")==0 )
      mon=10;
   else if ( strcmp(smon, "Nov")==0 )
      mon=11;
   else if ( strcmp(smon, "Dec")==0 )
      mon=12;
   else
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
      
   //The day chack is a consistent check that could be dropped.
   if ( strcmp(sday, "Sun")==0 )
      weekDay=0;
   else if ( strcmp(sday, "Mon")==0 )
      weekDay=1;
   else if ( strcmp(sday, "Tue")==0 )
      weekDay=2;
   else if ( strcmp(sday, "Wed")==0 )
      weekDay=3;
   else if ( strcmp(sday, "Thu")==0 )
      weekDay=4;
   else if ( strcmp(sday, "Fri")==0 )
      weekDay=5;
   else if ( strcmp(sday, "Sat")==0 )
      weekDay=6;
   else
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
                  
   if( day<0 || day>31 || h<0 || h>23 || m<0 || m>59 || s<0 || s>60 )
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );   
      
   boost::posix_time::ptime date( boost::gregorian::date(year, mon, day) , 
                                  boost::posix_time::time_duration( h, m, s )  );
   
   //This is a consistent check. We could skip this test.
   if( weekDay != date.date().day_of_week() )
      return boost::posix_time::ptime( boost::date_time::not_a_date_time );
      
   return date;
}

std::string miutil::isotimeString(const boost::posix_time::ptime &pt, bool useTseparator, bool isLocalFormat)
{ 
   char buf[32];
   char sep=' '; 
      
   if( pt.is_special() )
      return "";
   
   if( useTseparator )
      sep = 'T';
   
   boost::gregorian::date d( pt.date() );
   boost::posix_time::time_duration t( pt.time_of_day() );

   if( !isLocalFormat ){
   	sprintf( buf, "%04d-%02d-%02d%c%02d:%02d:%02d.%03dZ", 
             static_cast<unsigned short>(d.year()), d.month().as_number(), d.day().as_number(), sep,
             t.hours(), t.minutes(), t.seconds(),(static_cast<int64_t>(t.total_milliseconds()))%1000 );
   }
   else{
       boost::posix_time::ptime timeP(d,t);

       char utc_buff[8]={0};
       time_t rawtime;
       time ( &rawtime );
       struct tm *ptm = std::gmtime ( &rawtime );
       int utcH = ptm->tm_hour;
       int utcM = ptm->tm_min;
       struct tm *timeinfo = std::localtime (&rawtime);
       int diffH = timeinfo->tm_hour - utcH;
       int diffM = timeinfo->tm_min - utcM;
       boost::posix_time::time_duration diffT = boost::posix_time::minutes(diffH*60+diffM);
       timeP += diffT;

       d = timeP.date();
       t = timeP.time_of_day();

       sprintf(utc_buff,diffT.hours()>0?"+%02d:%02d":"%03d:%02d",diffT.hours(),abs(diffM));
   	   sprintf( buf, "%04d-%02d-%02d%c%02d:%02d:%02d.%03d", 
               static_cast<unsigned short>(d.year()), d.month().as_number(), d.day().as_number(), sep,
               t.hours(), t.minutes(), t.seconds(),(static_cast<int64_t>(t.total_milliseconds()))%1000);
       strcat(buf,utc_buff);
   }
   
   return buf;   
}

boost::posix_time::ptime miutil::ptimeFromIsoString( const std::string &isoTime )
{
	struct DEF {
		int number;
		int numberOfChars;
		const char *nextSep;
	};
	
	DEF def[] = {{0, 4, "-"},  //year
			       {0, 2, "-"},  //Month
			       {0, 2, "T "},//day
			       {0, 2, ":"}, //hour
			       {0, 2, ":"}, //minute
			       {0, 2, ".Z"}, //second
                   {0, 3, "+-Z"}, //msecond
			       {0, 0, 0}    //terminator
			      };
	
	int hourOffset=0;
	int minuteOffset=0;
    string::size_type iIsoTime=0;
	
	if( isoTime == "infinity" )
		return boost::posix_time::ptime( boost::posix_time::pos_infin );
	else if( isoTime == "-infinity" )
		return boost::posix_time::ptime( boost::posix_time::neg_infin );
	else if( isoTime == "epoch" )
		return boost::posix_time::ptime( boost::gregorian::date( 1970, 1, 1 ),
		       			                 boost::posix_time::time_duration( 0, 0, 0) );
	else if( isoTime == "now" )
		return boost::posix_time::second_clock::universal_time();
	else if( isoTime == "today" ) {
		boost::posix_time::ptime now( boost::posix_time::second_clock::universal_time() );
		return boost::posix_time::ptime( now.date(), boost::posix_time::time_duration( 0, 0, 0) );
	} else if( isoTime == "tomorrow" ) {
		boost::posix_time::ptime now( boost::posix_time::second_clock::universal_time() );
		now += boost::posix_time::hours(24);
		return boost::posix_time::ptime( now.date(), boost::posix_time::time_duration( 0, 0, 0) );
	} else if( isoTime == "yesterday" ) {
		boost::posix_time::ptime now( boost::posix_time::second_clock::universal_time() );
		now -= boost::posix_time::hours(24);
		return boost::posix_time::ptime( now.date(), boost::posix_time::time_duration( 0, 0, 0) );
	}

	iIsoTime = isoTime.find_first_not_of( " ", 0 ); //Skip whitespace in front
	
	assert( iIsoTime != string::npos );
	
	if( ! isdigit( (int)isoTime[iIsoTime] ) ) {
		//We try to decode it as an rfc1123date.
		boost::posix_time::ptime pt = rfc1123date( isoTime.c_str() );
		
		assert( !pt.is_special() );
		
		return pt;
	}
	
	//Decode the YYYY-MM-DDThh:mm:ss part
	//Remeber -, T and : is optional.
	for( int defIndex=0; def[defIndex].nextSep && iIsoTime < isoTime.length(); ++defIndex ) {
		def[defIndex].number = getInt( isoTime, iIsoTime, def[defIndex].numberOfChars );	
		iIsoTime = isoTime.find_first_of( def[defIndex].nextSep, iIsoTime );
        if(defIndex!=6)
           iIsoTime++;
	}

	//Decode the UTC offset part of
	//YYYY-MM-DDThh:mm:ssSHHMM part
	//The UTC offset part is SHHMM
	//Where S is the sign. + or -.
	//      HH the hour offset, mandatory if we have an UTC offset part.
	//      MM the minute offset, otional.
	if( iIsoTime != string::npos && iIsoTime < isoTime.length() ) {
		//We have a possible UTC offset.
		//The format is SHHMM, where S is the sign.
		char ch = isoTime[iIsoTime];
		
		if( ch == 'z' || ch == 'Z' ) { 
			iIsoTime = string::npos;
		} else if( ch == '+' || ch == '-' || isdigit( ch ) ) {
			int sign=1;
			
			if( ! isdigit( ch ) ) {
				iIsoTime++;
				if( ch=='-' )
					sign=-1;
				assert( isdigit( isoTime[iIsoTime] ) );
			}
			hourOffset = getInt( isoTime, iIsoTime, 2 );
			hourOffset*=sign;
			
			if( iIsoTime < isoTime.length() && isdigit( isoTime[++iIsoTime] ) ) {
				minuteOffset = getInt( isoTime, iIsoTime, 2 );
			}
		}
	}
						
	boost::gregorian::date date(def[0].number, def[1].number, def[2].number);
	boost::posix_time::time_duration td(def[3].number, def[4].number, def[5].number,def[6].number*1000);
	boost::posix_time::ptime pt(date, td);
	
	td=boost::posix_time::time_duration(hourOffset, minuteOffset, 0);
	
	return pt - td;
}
	

namespace {
	
	/**
	 * @return The int on success.
	 * 
	 *        0 if there is no more characters to read from timebuf.
	 *       -1 if an invalid character was encontred.
	 */
	int getInt( const string &timebuf, string::size_type &index, int numberOfChar )
	{
		int nextN=0;
        char buf[10]={0};
		assert(numberOfChar < 9);		
		while( nextN < numberOfChar ) {
			assert( index < timebuf.length() );
			char n = timebuf[index];			
			assert(isdigit( n ) );			
			buf[nextN]=n;
			nextN++;
			index++;
		}
		return atoi( buf );
	}
	
	
	
}


