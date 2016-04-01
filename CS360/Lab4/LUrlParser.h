#pragma once

#include <string>

namespace LUrlParser
{
    enum LUrlParserError
    {
        LUrlParserError_Ok = 0,
        LUrlParserError_Uninitialized = 1,
        LUrlParserError_NoUrlCharacter = 2,
        LUrlParserError_InvalidSchemeName = 3,
        LUrlParserError_NoDoubleSlash = 4,
        LUrlParserError_NoAtSign = 5,
        LUrlParserError_UnexpectedEndOfLine = 6,
        LUrlParserError_NoSlash = 7,
    };
    
    class clParseURL
    {
    public:
        LUrlParserError m_ErrorCode;
        std::string m_Scheme;
        std::string m_Host;
        std::string m_Port;
        std::string m_Path;
        std::string m_Query;
        std::string m_Fragment;
        std::string m_UserName;
        std::string m_Password;
        
        clParseURL()
        : m_ErrorCode( LUrlParserError_Uninitialized )
        {}
        
        /// return 'true' if the parsing was successful
        bool IsValid() const { return m_ErrorCode == LUrlParserError_Ok; }
        
        /// helper to convert the port number to int, return 'true' if the port is valid (within the 0..65535 range)
        bool GetPort( int* OutPort ) const;
        
        /// parse the URL
        static clParseURL ParseURL( const std::string& URL );
        
    private:
        explicit clParseURL( LUrlParserError ErrorCode )
        : m_ErrorCode( ErrorCode )
        {}
    };
    
}

