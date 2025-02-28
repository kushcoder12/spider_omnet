//
// Generated file, do not edit! Created by nedtool 5.6 from priceQueryMsg.msg.
//

#ifndef __PRICEQUERYMSG_M_H
#define __PRICEQUERYMSG_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include <vector>
#include <map>
typedef std::vector<int> IntVector;
typedef std::vector<double> DoubleVector;
typedef std::map<int, double> IntMap;
// }}

/**
 * Class generated from <tt>priceQueryMsg.msg:29</tt> by nedtool.
 * <pre>
 * packet priceQueryMsg
 * {
 *     double zValue = 0;
 *     int pathIndex = 0;
 *     bool isReversed = false;
 * }
 * </pre>
 */
class priceQueryMsg : public ::omnetpp::cPacket
{
  protected:
    double zValue;
    int pathIndex;
    bool isReversed;

  private:
    void copy(const priceQueryMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const priceQueryMsg&);

  public:
    priceQueryMsg(const char *name=nullptr, short kind=0);
    priceQueryMsg(const priceQueryMsg& other);
    virtual ~priceQueryMsg();
    priceQueryMsg& operator=(const priceQueryMsg& other);
    virtual priceQueryMsg *dup() const override {return new priceQueryMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual double getZValue() const;
    virtual void setZValue(double zValue);
    virtual int getPathIndex() const;
    virtual void setPathIndex(int pathIndex);
    virtual bool getIsReversed() const;
    virtual void setIsReversed(bool isReversed);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const priceQueryMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, priceQueryMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __PRICEQUERYMSG_M_H

