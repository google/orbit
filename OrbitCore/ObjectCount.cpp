#include "ObjectCount.h"

ObjectCounter GObjectCounter;

//-----------------------------------------------------------------------------
int ObjectCounter::Inc(const char* a_ObjectType) {
  ScopeLock lock(m_Mutex);
  ++m_ObjectCount[a_ObjectType];
  return m_ObjectCount[a_ObjectType];
}

//-----------------------------------------------------------------------------
int ObjectCounter::Dec(const char* a_ObjectType) {
  ScopeLock lock(m_Mutex);
  --m_ObjectCount[a_ObjectType];
  return m_ObjectCount[a_ObjectType];
}

//-----------------------------------------------------------------------------
int ObjectCounter::Count(const char* a_ObjecType) {
  ScopeLock lock(m_Mutex);
  return m_ObjectCount[a_ObjecType];
}