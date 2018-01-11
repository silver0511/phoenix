// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_FBGROUPCREATERQ_GROUPPACK_H_
#define FLATBUFFERS_GENERATED_FBGROUPCREATERQ_GROUPPACK_H_

#include "flatbuffers/flatbuffers.h"

#include "common_generated.h"
#include "fb_group_info_generated.h"
#include "fb_group_offline_msg_generated.h"

namespace grouppack {

struct T_GROUP_CREATE_RQ;

struct T_GROUP_CREATE_RQ FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_S_RQ_HEAD = 4,
    VT_GROUP_NAME = 6,
    VT_GROUP_IMG_URL = 8,
    VT_GROUP_REMARK = 10,
    VT_GROUP_CT = 12,
    VT_GROUP_TYPE = 14,
    VT_OFFLINE_GROUP_MSG = 16
  };
  const commonpack::S_RQ_HEAD *s_rq_head() const { return GetStruct<const commonpack::S_RQ_HEAD *>(VT_S_RQ_HEAD); }
  const flatbuffers::String *group_name() const { return GetPointer<const flatbuffers::String *>(VT_GROUP_NAME); }
  const flatbuffers::String *group_img_url() const { return GetPointer<const flatbuffers::String *>(VT_GROUP_IMG_URL); }
  const flatbuffers::String *group_remark() const { return GetPointer<const flatbuffers::String *>(VT_GROUP_REMARK); }
  int64_t group_ct() const { return GetField<int64_t>(VT_GROUP_CT, 0); }
  int32_t group_type() const { return GetField<int32_t>(VT_GROUP_TYPE, 0); }
  const grouppack::T_OFFLINE_GROUP_MSG *offline_group_msg() const { return GetPointer<const grouppack::T_OFFLINE_GROUP_MSG *>(VT_OFFLINE_GROUP_MSG); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<commonpack::S_RQ_HEAD>(verifier, VT_S_RQ_HEAD) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_GROUP_NAME) &&
           verifier.Verify(group_name()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_GROUP_IMG_URL) &&
           verifier.Verify(group_img_url()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_GROUP_REMARK) &&
           verifier.Verify(group_remark()) &&
           VerifyField<int64_t>(verifier, VT_GROUP_CT) &&
           VerifyField<int32_t>(verifier, VT_GROUP_TYPE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_OFFLINE_GROUP_MSG) &&
           verifier.VerifyTable(offline_group_msg()) &&
           verifier.EndTable();
  }
};

struct T_GROUP_CREATE_RQBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_s_rq_head(const commonpack::S_RQ_HEAD *s_rq_head) { fbb_.AddStruct(T_GROUP_CREATE_RQ::VT_S_RQ_HEAD, s_rq_head); }
  void add_group_name(flatbuffers::Offset<flatbuffers::String> group_name) { fbb_.AddOffset(T_GROUP_CREATE_RQ::VT_GROUP_NAME, group_name); }
  void add_group_img_url(flatbuffers::Offset<flatbuffers::String> group_img_url) { fbb_.AddOffset(T_GROUP_CREATE_RQ::VT_GROUP_IMG_URL, group_img_url); }
  void add_group_remark(flatbuffers::Offset<flatbuffers::String> group_remark) { fbb_.AddOffset(T_GROUP_CREATE_RQ::VT_GROUP_REMARK, group_remark); }
  void add_group_ct(int64_t group_ct) { fbb_.AddElement<int64_t>(T_GROUP_CREATE_RQ::VT_GROUP_CT, group_ct, 0); }
  void add_group_type(int32_t group_type) { fbb_.AddElement<int32_t>(T_GROUP_CREATE_RQ::VT_GROUP_TYPE, group_type, 0); }
  void add_offline_group_msg(flatbuffers::Offset<grouppack::T_OFFLINE_GROUP_MSG> offline_group_msg) { fbb_.AddOffset(T_GROUP_CREATE_RQ::VT_OFFLINE_GROUP_MSG, offline_group_msg); }
  T_GROUP_CREATE_RQBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  T_GROUP_CREATE_RQBuilder &operator=(const T_GROUP_CREATE_RQBuilder &);
  flatbuffers::Offset<T_GROUP_CREATE_RQ> Finish() {
    auto o = flatbuffers::Offset<T_GROUP_CREATE_RQ>(fbb_.EndTable(start_, 7));
    return o;
  }
};

inline flatbuffers::Offset<T_GROUP_CREATE_RQ> CreateT_GROUP_CREATE_RQ(flatbuffers::FlatBufferBuilder &_fbb,
    const commonpack::S_RQ_HEAD *s_rq_head = 0,
    flatbuffers::Offset<flatbuffers::String> group_name = 0,
    flatbuffers::Offset<flatbuffers::String> group_img_url = 0,
    flatbuffers::Offset<flatbuffers::String> group_remark = 0,
    int64_t group_ct = 0,
    int32_t group_type = 0,
    flatbuffers::Offset<grouppack::T_OFFLINE_GROUP_MSG> offline_group_msg = 0) {
  T_GROUP_CREATE_RQBuilder builder_(_fbb);
  builder_.add_group_ct(group_ct);
  builder_.add_offline_group_msg(offline_group_msg);
  builder_.add_group_type(group_type);
  builder_.add_group_remark(group_remark);
  builder_.add_group_img_url(group_img_url);
  builder_.add_group_name(group_name);
  builder_.add_s_rq_head(s_rq_head);
  return builder_.Finish();
}

inline flatbuffers::Offset<T_GROUP_CREATE_RQ> CreateT_GROUP_CREATE_RQDirect(flatbuffers::FlatBufferBuilder &_fbb,
    const commonpack::S_RQ_HEAD *s_rq_head = 0,
    const char *group_name = nullptr,
    const char *group_img_url = nullptr,
    const char *group_remark = nullptr,
    int64_t group_ct = 0,
    int32_t group_type = 0,
    flatbuffers::Offset<grouppack::T_OFFLINE_GROUP_MSG> offline_group_msg = 0) {
  return CreateT_GROUP_CREATE_RQ(_fbb, s_rq_head, group_name ? _fbb.CreateString(group_name) : 0, group_img_url ? _fbb.CreateString(group_img_url) : 0, group_remark ? _fbb.CreateString(group_remark) : 0, group_ct, group_type, offline_group_msg);
}

inline const grouppack::T_GROUP_CREATE_RQ *GetT_GROUP_CREATE_RQ(const void *buf) {
  return flatbuffers::GetRoot<grouppack::T_GROUP_CREATE_RQ>(buf);
}

inline bool VerifyT_GROUP_CREATE_RQBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<grouppack::T_GROUP_CREATE_RQ>(nullptr);
}

inline void FinishT_GROUP_CREATE_RQBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<grouppack::T_GROUP_CREATE_RQ> root) {
  fbb.Finish(root);
}

}  // namespace grouppack

#endif  // FLATBUFFERS_GENERATED_FBGROUPCREATERQ_GROUPPACK_H_