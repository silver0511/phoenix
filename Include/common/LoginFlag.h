#ifndef __LOGINFLAG_H_
#define __LOGINFLAG_H_

#include <bitset>
#include <string>

using namespace std;

template<class T>
class CLoginFlag
{
private:
    bitset<64> moState;
	int64 miSuccessFalg;
public:

    inline CLoginFlag();

    inline void SetState(size_t aiStateType, bool abStateValue);

    inline bool GetState(size_t aiStateType) const;

    inline void ResetState(bool abStateValue);

    inline bool IsAllSuccess() const;

    inline string GetLogString() const;
};

template<class T>
CLoginFlag<T>::CLoginFlag()
{
	miSuccessFalg = T::LOGIN_FLAG_SUM;
	miSuccessFalg = ((int64)1 <<miSuccessFalg) - 1;
    for (int32 i = 0; i < T::LOGIN_FLAG_SUM; i++)
    {
        moState.set(i, true);
    }
}

template<class T>
bool CLoginFlag<T>::IsAllSuccess() const
{
    for (int32 i = 0; i < T::LOGIN_FLAG_SUM; i++)
    {
        if (moState.test(i) != true)
        {
            return false;
        }
    }
    return true;
}

template<class T>
void CLoginFlag<T>::SetState(size_t aiStateType, bool abStateValue)
{
    moState.set(aiStateType, abStateValue);
}

template<class T>
void CLoginFlag<T>::ResetState(bool abStateValue)
{
    for (int32 i = 0; i < T::LOGIN_FLAG_SUM; i++)
    {
        moState.set(i, abStateValue);
    }
    return;
}

template<class T>
bool CLoginFlag<T>::GetState(size_t aiStateType) const
{
    return moState.test(aiStateType);
}

template<class T>
string CLoginFlag<T>::GetLogString() const
{
    char lszTemp[T::LOGIN_FLAG_SUM + 1];
    lszTemp[T::LOGIN_FLAG_SUM] = '\0';

    for(int i = 0; i < T::LOGIN_FLAG_SUM; i++)
    {
        if(moState[(T::LOGIN_FLAG_SUM - 1) - i])
        {
            lszTemp[i]='1';
        }
        else
        {
            lszTemp[i]='0';
        }
    }
    string lstrTemp = lszTemp;
    return lstrTemp;
}

#endif //__LOGINFLAG_H_