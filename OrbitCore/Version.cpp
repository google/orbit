#include "Version.h"
#include "Utils.h"
#include "Core.h"
#include "curl/curl.h"

//-----------------------------------------------------------------------------
bool        OrbitVersion::s_NeedsUpdate;
std::string OrbitVersion::s_LatestVersion;

#define OrbitVersionStr "dev"

//-----------------------------------------------------------------------------
std::string OrbitVersion::GetVersion()
{
    return OrbitVersionStr;
}

//-----------------------------------------------------------------------------
bool OrbitVersion::IsDev()
{
    return GetVersion() == std::string("dev");
}

//-----------------------------------------------------------------------------
static size_t WriteCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    ( ( std::string* )userp )->append( (char*)contents, size * nmemb );
    return size * nmemb;
}

//-----------------------------------------------------------------------------
void OrbitVersion::CheckForUpdate()
{
    if( !IsDev() )
    {
        std::thread* thread = new std::thread( [&](){ CheckForUpdateThread(); } );
        thread->detach();
    }
}

//-----------------------------------------------------------------------------
void OrbitVersion::CheckForUpdateThread()
{
    CURL *curl;
    std::string readBuffer;

    curl = curl_easy_init();
    if( curl )
    {
        curl_easy_setopt( curl, CURLOPT_URL, "http://www.telescopp.com/update" );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &readBuffer );
        curl_easy_perform( curl );
        curl_easy_cleanup( curl );

        // Get latest version from html, this needs to match what is on the website...
        std::string searchStr = "Latest version of the Orbit Profiler is: ";
        size_t pos = readBuffer.find( searchStr );

        if( pos != std::string::npos )
        {
            std::string version = Replace( readBuffer.substr( pos, 60 ), searchStr, "" );
            std::vector< std::string > tokens = Tokenize( version );

            if( tokens.size() > 0 )
            {
                s_LatestVersion = tokens[0];
                s_NeedsUpdate = s_LatestVersion != GetVersion();
            }
        }
    }
}
