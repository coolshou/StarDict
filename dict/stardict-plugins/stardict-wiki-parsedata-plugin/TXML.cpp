/*
 * Copyright 2011 kubtek <kubtek@mail.com>
 *
 * This file is part of StarDict.
 *
 * StarDict is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StarDict is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StarDict.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TXML.h"

// *****************************************************************************
// *****************************************************************************
//
// TXML
//
// *****************************************************************************
// *****************************************************************************

TXML::TXML ( int f , int t , string &s , bool fix_comments )
	{
	from = f ;
	to = t ;
	name = s.substr ( from + 1 , to - (from+1) ) ;
	name = trim ( name ) ;
	name = before_first ( ' ' , name ) ;
	closing = selfclosing = false ;
	if ( left ( name , 1 ) == "/" )
	   {
 	   closing = true ;
 	   name = name.substr ( 1 , name.length()-1 ) ;
 	   }
	if ( right ( name , 1 ) == "/" )
	   {
 	   selfclosing = true ;
 	   name = name.substr ( 0 , name.length()-1 ) ;
 	   }
    name = trim ( name ) ;
	
	// This will replace < and > within a comment with the appropriate HTML entities
	if ( fix_comments && left ( name , 1 ) == "!" )
		{
		int a ;
		for ( a = from+1 ; a < to ; a++ )
			{
			if ( s[a] != '>' && s[a] != '<' ) continue ;
			to += 3 ;
			if ( s[a] == '>' ) s.insert ( a , "&gt" ) ;
			if ( s[a] == '<' ) s.insert ( a , "&lt" ) ;
			s[a+3] = ';' ;
			}    
		}    
	}    

void TXML::remove_at ( int pos )
	{
	if ( pos < from ) from-- ;
	if ( pos < to ) to-- ;
	}    

void TXML::insert_at ( int pos )
	{
	if ( pos < from ) from++ ;
	if ( pos < to ) to++ ;
	}    

void TXML::add_key_value ( string k , string v )
	{
    key.push_back ( trim ( k ) ) ;
    value.push_back ( trim ( v ) ) ;
	}    
	
string TXML::get_string ()
	{
    string ret ;
    ret = "<" + name ;
    for ( size_t a = 0 ; a < key.size() ; a++ )
    	{
	    for ( size_t b = 0 ; b < key[a].length() ; b++ )
	    	{
      		if ( key[a][b] == ' ' ) key[a][b] = '_' ;
      		}  		
	    ret += " " + key[a] ;
	    if ( value[a] != "" ) ret += "=\"" + unquote ( SINGLE_QUOTE , value[a] ) + "\"" ;
    	}    
   	if ( text == "" ) ret += "/>" ;
   	else ret += ">" + text + "</" + name + ">" ;
   	return ret ;
	}
 

