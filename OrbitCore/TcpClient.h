//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "TcpEntity.h"
#include <vector>

class TcpClient : public TcpEntity
{
public:
    TcpClient( const std::string & a_Host );
    ~TcpClient();

    bool IsValid() const { return m_IsValid; }
    void Start() override;

protected:
    void ClientThread();
    void ReadMessage();
    void ReadPayload();
	void ReadFooter();
    void DecodeMessage( Message & a_Message );
    void OnError( const std::error_code& ec );
    virtual TcpSocket* GetSocket() override final { return m_TcpSocket; }

private:
    Message           m_Message;
    std::vector<char> m_Payload;
    bool              m_IsValid;
};

extern std::unique_ptr<TcpClient> GTcpClient;
