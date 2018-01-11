package com.qbao.im.api.handler;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.qbao.im.api.config.FriendConfiguration;
import com.qbao.im.api.constants.FriendBlackEnums;
import com.qbao.im.api.constants.FriendStatusEnums;
import com.qbao.im.api.entity.Friend;
import com.qbao.im.api.msg.ClientMessage;
import com.qbao.im.api.msg.ErrorCode;
import com.qbao.im.api.service.impl.FriendServiceImpl;

/**
 * Created by tangxiaojun on 2017/5/18. Updated by lishuguang on 2017/10/31
 */
@Service
public class FriendHandler {

	private Logger logger = LoggerFactory.getLogger(FriendHandler.class);

	@Autowired
	private FriendServiceImpl friendService;

	@Autowired
	private FriendConfiguration friendConfiguration;

	public List<Friend> getFriend(Long user_id, Integer pageIndex, Integer pageSize, Integer status) {
		return this.friendService.getFriends(user_id, pageIndex, pageSize, status);
	}

	/**
	 * 申请加好友
	 * 
	 * @param userId
	 *            申请人用户id
	 * @param friendId
	 *            对方用户id
	 * @param sourceType
	 * @param remarkName
	 *            对方昵称
	 * @param friendMsg
	 *            申请语
	 * @return
	 */
	public ClientMessage applyFriend(Long userId, Long friendId, Integer sourceType, String remarkName,
			String friendMsg) {
		ClientMessage success = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();

		Friend existSelfFriend = null;
		Friend existDestFriend = null;
		try {
			// 1. 获取已存在好友信息
			existSelfFriend = getFriendRelation(userId, friendId);
			existDestFriend = getFriendRelation(friendId, userId);

			// 2. 判断业务异常
			ClientMessage checkResult = checkOperValid(userId, friendId, existSelfFriend, existDestFriend);
			if (checkResult != null){
				return checkResult;
			}
			
			// 3. 检查双方好友上限
			if (checkFriendMaxLimit(userId, friendId, existSelfFriend) == false) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_MAX_NUM).build();
			}
			if (checkFriendMaxLimit(friendId, userId, existDestFriend) == false) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_DEST_MAX_NUM)
						.build();
			}

			// 3. 处理请求
			// 3.1 已方与对方无好友信息
			if (existSelfFriend == null) {
				// NA-NA-NA-NA --> 0-0-0-0
				if (existDestFriend == null) {
					int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
							FriendStatusEnums.TARGET_VALID.getValue(), true);
					r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
							FriendStatusEnums.FRIEND_VALID.getValue(), true);
					return success;
				} else { // 3.1.2对方与已方有好友信息
							// NA-NA-2-0 --> 2-0-3-0 3.1.2.1. 对方已有好友信息（待好友验证).
							// 结果： 已方待验证，对方验证好友
					if (FriendStatusEnums.TARGET_VALID.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.TARGET_VALID.getValue(), true);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), false);
						return success;
					}
					// 3.1.2.2 对方已有好友信息（被动删除）
					if (FriendStatusEnums.BEDELETE.getValue() == existDestFriend.getStatus().intValue()) {
						// NA-NA-7-0 --> 0-0-0-0 3.1.2.2.2 对方没有主动拉黑，则恢复好友,返回恢复好友
						if (FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
							int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
									FriendStatusEnums.FRIEND.getValue(), true);
							r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
									FriendStatusEnums.FRIEND.getValue(), false);
							return new ClientMessage.Builder().user_id(String.valueOf(userId))
									.responseCode(ErrorCode.RET_FRIEND_BE_DELETE_ERROR).build();
						}
					}
				}
			} else {// 3.2 已方有好友信息
				if (existDestFriend == null) {
					// 2-0-NA-NA --> 2-0-3-0
					if (FriendStatusEnums.TARGET_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.TARGET_VALID.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), true);
						return success;
					}
					// 7-0-NA-NA --> 11-0-3-0
					if (FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.BEDELETEAPPLY.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), true);
						return success;
					}
					// 3-0-NA-NA --> 0-0-0-0
					if (FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND.getValue(), true);
						return success;
					}
					// 11-0-NA-NA --> 11-0-3-0
					if (FriendStatusEnums.BEDELETEAPPLY.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.BEDELETEAPPLY.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), true);
						return success;
					}
				} else {
					// 3-0-2-0 --> 3-0-3-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.TARGET_VALID.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND_VALID.getValue(), false);
						return success;
					}
					// 3-0-11-0 --> 0-0-0-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.BEDELETEAPPLY.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						int r = saveFriendInfo(userId, friendId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND.getValue(), false);
						r = saveFriendInfo(friendId, userId, sourceType, remarkName, friendMsg,
								FriendStatusEnums.FRIEND.getValue(), false);
						return success;
					}
				}
			}

			logger.warn(
					"[SaveFriend]: no reachable situation! userId:{}, friendId:{}, existSelfFriend:{}, existDestFriend:{}",
					userId, friendId, existSelfFriend == null ? "null" : existSelfFriend.toString(),
					existDestFriend == null ? "null" : existDestFriend.toString());
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_ADD_ERROR).build();
		} catch (Exception e) {
			logger.error("[SaveFriend]: throw exception!  userId:{}, friendId:{}, e:{}", userId, friendId, e);
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_ADD_ERROR).build();
		}
	}
	
	/**
	 * 检查申请好友时状态是否异常
	 * @param userId
	 * @param friendId
	 * @param existSelfFriend
	 * @param existDestFriend
	 * @return null： 可正常加好友   非null： 可直接返回客户端
	 */
	private ClientMessage checkOperValid(Long userId, Long friendId, Friend existSelfFriend, Friend existDestFriend) {
		ClientMessage result = null;
		// NA-NA-7-2 --> NA-NA-7-2 3.1.2.2.1 对方主动拉黑，则返回异常
		if (existSelfFriend == null && existDestFriend != null
				&& FriendStatusEnums.BEDELETE.getValue() == existDestFriend.getStatus().intValue()
				&& FriendBlackEnums.BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAS_BLACK_ERROR)
					.build();
		}
		// 7-2-NA-NA --> 7-2-NA-NA 抛出异常，已在黑名单里面
		if (existSelfFriend != null && existDestFriend == null
				&& FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
				&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()) {
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAVE_BLACK_ERROR)
					.build();
		}
		
		// 11-2-NA-NA --> 11-2-NA-NA 抛出异常，已在黑名单里面
		if (existSelfFriend != null && existDestFriend == null
				&& FriendStatusEnums.BEDELETEAPPLY.getValue() == existSelfFriend.getStatus().intValue()
				&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()) {
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAVE_BLACK_ERROR)
					.build();
		}

		if (existSelfFriend != null && existDestFriend != null) {
			// 0-0-0-0 --> 0-0-0-0 抛异常，已为好友
			if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_ALREADY_EXISTED)
						.build();
			}
			// 0-1-0-2 --> 0-1-0-2 抛异常，对方主动拉黑
			if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAS_BLACK_ERROR)
						.build();
			}
			// 0-2-0-1 --> 0-2-0-1 抛异常，自己主动拉黑
			if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAVE_BLACK_ERROR)
						.build();
			}
			// 0-3-0-3 --> 0-3-0-3 抛异常，自己主动拉黑
			if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_HAVE_BLACK_ERROR)
						.build();
			}
			// 2-0-3-0 --> 2-0-3-0 重复申请，返回待对方处理
			if (FriendStatusEnums.TARGET_VALID.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND_VALID.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_RELATION_ERROR)
						.build();
			}
			// 11-0-3-0 --> 11-0-3-0 重复申请，返回待好友处理
			if (FriendStatusEnums.BEDELETEAPPLY.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND_VALID.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_RELATION_ERROR)
						.build();
			}
		}
		return result;
	}

	/**
	 * 保存指定好友信息
	 * 
	 * @param userId
	 * @param friendId
	 * @param sourceType
	 * @param remarkName
	 * @param friendMsg
	 * @param status
	 * @param isNew
	 * @return 数据库操作影响行数 为1为正常，否则 为不正常
	 */
	private int saveFriendInfo(Long userId, Long friendId, Integer sourceType, String remarkName, String friendMsg,
			Integer status, boolean isNew) throws Exception {
		Friend f = new Friend(userId, friendId, sourceType, remarkName);
		if (friendMsg != null) {
			f.setFriendMsg(friendMsg);
		}
		f.setStatus(status);
		if (isNew) {
			return this.friendService.save(userId, f);
		} else {
			return this.friendService.modifyFriend(userId, f);
		}
	}

	/**
	 * 获取指定一个用户与另外一个用户的好友信息
	 * 
	 * @param userId
	 * @param friendId
	 * @return
	 */
	private Friend getFriendRelation(Long userId, Long friendId) {
		List<Friend> friendList = this.friendService.getFriends(userId, friendId);
		if (friendList == null || friendList.isEmpty()) {
			return null;
		} else if (friendList.size() > 1) {
			logger.error("More than 1 friend records found with userId:{}, friendId:{}", userId, friendId);
			throw new RuntimeException("DB unique key duplicated in nim_friend table");
		}
		return friendList.get(0);
	}

	/**
	 * 根据要加好友信息判断好友上限
	 * 
	 * @param userId
	 * @param friendId
	 * @param existFriend
	 *            已存在好友
	 * @return true: 没达到上限 false: 达到上限
	 */
	private boolean checkFriendMaxLimit(Long userId, Long friendId, Friend existFriend) {
		int friendCount = 0;
		// 获取用户当前好友数
		if (existFriend == null) {
			Friend p = new Friend(userId, friendId);
			friendCount = friendService.getFriendMaxNum(userId, p);
		} else {
			friendCount = friendService.getFriendMaxNum(userId, existFriend);
		}

		// 当前好友数不到上限，返回true
		if (friendCount < friendConfiguration.getLimit()) {
			return true;
		}

		// 如果当前好友关系是被删除或是删除后添加，则直接返回true， 因为被删除和删除后添加是会占有好友数量的
		if (existFriend != null && (FriendStatusEnums.BEDELETE.getValue() == existFriend.getStatus().intValue()
				|| FriendStatusEnums.BEDELETEAPPLY.getValue() == existFriend.getStatus().intValue())) {
			return true;
		}

		// 如果当前好友数达到（或超过）好友上限,返回false
		if (friendCount >= friendConfiguration.getLimit().intValue()) {
			return false;
		}

		return false;
	}

	public boolean del(Long userId, Long friendId) {
		try {
			// 1. 获取已有好友信息
			Friend existSelfFriend = getFriendRelation(userId, friendId);
			Friend existDestFriend = getFriendRelation(friendId, userId);

			// 2. 根据已有好友信息做处理
			if (existSelfFriend != null) {
				// 2.1 对方已有好友信息
				if (existDestFriend != null) {
					// 0-0-0-0 --> NA-NA-7-0
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.BEDELETE.getValue());
						r = friendService.modifyFriend(friendId, p);
						return true;
					}

					// 0-2-0-1 --> NA-NA-7-0
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existDestFriend.getIsBlackList()
									.intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.BEDELETE.getValue());
						p.setIsBlackList(FriendBlackEnums.NORMAL.getValue());
						r = friendService.modifyFriend(friendId, p);
						return true;
					}

					// 0-3-0-3 --> NA-NA-7-2
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList()
									.intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existDestFriend.getIsBlackList()
									.intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.BEDELETE.getValue());
						p.setIsBlackList(FriendBlackEnums.BLACK_LIST.getValue());
						r = friendService.modifyFriend(friendId, p);
						return true;
					}

					// 0-1-0-2 --> NA-NA-7-2
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.BEDELETE.getValue());
						r = friendService.modifyFriend(friendId, p);
						return true;
					}

					// 11-0-3-0 --> NA-NA-3-0
					if (FriendStatusEnums.BEDELETEAPPLY.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND_VALID.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						return true;
					}
				} else { // 对方无好友信息
					// 7-0-NA-NA --> NA-NA-NA-NA
					if (FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						return true;
					}

					// 7-2-NA-NA --> NA-NA-NA-NA
					if (FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						return true;
					}

					// 11-0-NA-NA --> NA-NA-NA-NA
					if (FriendStatusEnums.BEDELETEAPPLY.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						int r = friendService.delFriend(userId, p);
						return true;
					}
				}
			}

			logger.warn(
					"[DelFriend]: no reachable situation! userId:{}, friendId:{}, existSelfFriend:{}, existDestFriend:{}",
					userId, friendId, existSelfFriend == null ? "null" : existSelfFriend.toString(),
					existDestFriend == null ? "null" : existDestFriend.toString());
			return false;
		} catch (Exception e) {
			logger.error("[DelFriend]: throws exception!  userId:{}, friendId:{}, e:{}", userId, friendId, e);
			return false;
		}
	}

	/**
	 * 删除好友申请
	 * 
	 * @param userId
	 * @param friendId
	 * @return
	 */
	public boolean deleteApply(Long userId, Long friendId) {
		try {
			// 1. 获取已有好友信息
			Friend existSelfFriend = getFriendRelation(userId, friendId);
			Friend existDestFriend = getFriendRelation(friendId, userId);

			if (existSelfFriend != null && existDestFriend != null) {
				// 3-0-2-0 --> NA-NA-2-0
				if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
						&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
						&& FriendStatusEnums.TARGET_VALID.getValue() == existDestFriend.getStatus().intValue()
						&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
					Friend p = new Friend(userId, friendId);
					int r = friendService.delFriend(userId, p);
					return true;
				}

				// 3-0-11-0 --> NA-NA-11-0
				if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
						&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
						&& FriendStatusEnums.BEDELETEAPPLY.getValue() == existDestFriend.getStatus().intValue()
						&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
					Friend p = new Friend(userId, friendId);
					int r = friendService.delFriend(userId, p);
					return true;
				}
			}

			// 3-0-NA-NA --> NA-NA-NA-NA
			if (existDestFriend == null && existSelfFriend != null
					&& FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
				Friend p = new Friend(userId, friendId);
				int r = friendService.delFriend(userId, p);
				return true;
			}

			logger.warn(
					"[DeleteApply]: no reachable situation! userId:{}, friendId:{}, existSelfFriend:{}, existDestFriend:{}",
					userId, friendId, existSelfFriend == null ? "null" : existSelfFriend.toString(),
					existDestFriend == null ? "null" : existDestFriend.toString());
			return false;
		} catch (Exception e) {
			logger.error("[DeleteApply]: throws exception!  userId:{}, friendId:{}, e:{}", userId, friendId, e);
			return false;
		}
	}

	/**
	 * 修改备注
	 * 
	 * @param friend
	 * @return
	 */
	public boolean modifyRemark(Friend friend) {
		try {
			int result = this.friendService.modifyFriend(friend.getUserId(), friend);
			if (result > 0) {
				return true;
			}
		} catch (Exception e) {
			logger.error("[ModifyRemark] throws exception!  user_id:{}, friendId:{}, e:{}", friend.getUserId(),
					friend.getFriendId(), e);
			return false;
		}
		return false;
	}

	/**
	 * 同意好友申请
	 * 
	 * @param userId
	 * @param friendId
	 * @param sourceType
	 * @return
	 */
	public ClientMessage agree(Long userId, Long friendId, Integer sourceType) {
		ClientMessage success = new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_SUCCESS).build();
		try {
			// 1. 获取已存在好友信息
			Friend existSelfFriend = getFriendRelation(userId, friendId);
			Friend existDestFriend = getFriendRelation(friendId, userId);

			// 2. 检查业务异常
			// 0-0-0-0 --> 0-0-0-0 抛异常“已是好友”
			if (existSelfFriend != null && existDestFriend != null
					&& FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
					&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
					&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_ALREADY_EXISTED)
						.build();
			}

			// 3. 检查双方好友上限
			if (checkFriendMaxLimit(userId, friendId, existSelfFriend) == false) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_MAX_NUM).build();
			}
			if (checkFriendMaxLimit(friendId, userId, existDestFriend) == false) {
				return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_DEST_MAX_NUM)
						.build();
			}

			// 处理逻辑
			if (existSelfFriend != null) {
				if (existDestFriend != null) {
					// 3-0-2-0 --> 0-0-0-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.TARGET_VALID.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						int r = friendService.modifyFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						r = friendService.modifyFriend(friendId, p);
						return success;
					}

					// 3-0-3-0 --> 0-0-0-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND_VALID.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						int r = friendService.modifyFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						r = friendService.modifyFriend(friendId, p);
						return success;
					}

					// 3-0-11-0 --> 0-0-0-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.BEDELETEAPPLY.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						int r = friendService.modifyFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						r = friendService.modifyFriend(friendId, p);
						return success;
					}

				} else {
					// 3-0-NA-NA --> 0-0-0-0
					if (FriendStatusEnums.FRIEND_VALID.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()) {
						Friend p = new Friend(userId, friendId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setSourceType(sourceType);
						int r = friendService.modifyFriend(userId, p);
						p = new Friend(friendId, userId);
						p.setStatus(FriendStatusEnums.FRIEND.getValue());
						p.setFriendMsg(existSelfFriend.getFriendMsg());
						p.setIsBlackList(FriendBlackEnums.NORMAL.getValue());
						p.setRemarkName(existSelfFriend.getRemarkName());
						p.setSourceType(sourceType);
						r = friendService.save(friendId, p);
						return success;
					}
				}
			}
			logger.warn(
					"[AgreeeApply]: no reachable situation! userId:{}, friendId:{}, existSelfFriend:{}, existDestFriend:{}",
					userId, friendId, existSelfFriend == null ? "null" : existSelfFriend.toString(),
					existDestFriend == null ? "null" : existDestFriend.toString());
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_AGREE_ERROR).build();
		} catch (Exception e) {
			logger.error("[AgreeApply]: throws exception! userId:{},friendId:, e:{}", userId, friendId, e);
			return new ClientMessage.Builder().user_id(String.valueOf(userId)).responseCode(ErrorCode.RET_FRIEND_AGREE_ERROR).build();
		}
	}


	/**
	 * 设置 黑名单
	 * 
	 * @param userId
	 * @param friendId
	 * @param type
	 *            0：取消黑名单 1：设为黑名单
	 * @return
	 */
	public boolean settingBlack(Long userId, Long friendId, Integer type) {
		try {
			// this.friendService.modifyBlack(userId, new Friend(userId,
			// friendId), type);
			// 1. 获取已存在好友信息
			Friend existSelfFriend = getFriendRelation(userId, friendId);
			Friend existDestFriend = getFriendRelation(friendId, userId);

			// 取消黑名单
			if (type.intValue() == 0) {
				if (existSelfFriend != null) {
					if (existDestFriend == null) {
						// 7-2-NA-NA --> 7-0-NA-NA
						if (FriendStatusEnums.BEDELETE.getValue() == existSelfFriend.getStatus().intValue()
								&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList()
										.intValue()) {
							existSelfFriend.setIsBlackList(FriendBlackEnums.NORMAL.getValue());
							int r = friendService.modifyFriend(userId, existSelfFriend);
							return true;
						}
					} else {
						// 0-3-0-3 --> 0-1-0-2
						if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
								&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList()
										.intValue()
								&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
								&& FriendBlackEnums.EACH_BLACK_LIST.getValue() == existDestFriend.getIsBlackList()
										.intValue()) {
							existSelfFriend.setIsBlackList(FriendBlackEnums.BE_BLACK_LIST.getValue());
							int r = friendService.modifyFriend(userId, existSelfFriend);
							existDestFriend.setIsBlackList(FriendBlackEnums.BLACK_LIST.getValue());
							r = friendService.modifyFriend(friendId, existDestFriend);
							return true;
						}
						// 0-2-0-1 --> 0-0-0-0
						if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
								&& FriendBlackEnums.BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
								&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
								&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existDestFriend.getIsBlackList()
										.intValue()) {
							existSelfFriend.setIsBlackList(FriendBlackEnums.NORMAL.getValue());
							int r = friendService.modifyFriend(userId, existSelfFriend);
							existDestFriend.setIsBlackList(FriendBlackEnums.NORMAL.getValue());
							r = friendService.modifyFriend(friendId, existDestFriend);
							return true;
						}
					}
				}

			} else if (type.intValue() == 1) { // 设置黑名单
				if (existSelfFriend != null && existDestFriend != null) {
					// 0-0-0-0 --> 0-2-0-1
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.NORMAL.getValue() == existDestFriend.getIsBlackList().intValue()) {
						existSelfFriend.setIsBlackList(FriendBlackEnums.BLACK_LIST.getValue());
						int r = friendService.modifyFriend(userId, existSelfFriend);
						existDestFriend.setIsBlackList(FriendBlackEnums.BE_BLACK_LIST.getValue());
						r = friendService.modifyFriend(friendId, existDestFriend);
						return true;
					}

					// 0-1-0-2 --> 0-3-0-3
					if (FriendStatusEnums.FRIEND.getValue() == existSelfFriend.getStatus().intValue()
							&& FriendBlackEnums.BE_BLACK_LIST.getValue() == existSelfFriend.getIsBlackList().intValue()
							&& FriendStatusEnums.FRIEND.getValue() == existDestFriend.getStatus().intValue()
							&& FriendBlackEnums.BLACK_LIST.getValue() == existDestFriend.getIsBlackList().intValue()) {
						existSelfFriend.setIsBlackList(FriendBlackEnums.EACH_BLACK_LIST.getValue());
						int r = friendService.modifyFriend(userId, existSelfFriend);
						existDestFriend.setIsBlackList(FriendBlackEnums.EACH_BLACK_LIST.getValue());
						r = friendService.modifyFriend(friendId, existDestFriend);
						return true;
					}
				}
			}

			logger.warn(
					"[SettingBlack]: no reachable situation! userId:{}, friendId:{}, type:{}, existSelfFriend:{}, existDestFriend:{}",
					userId, friendId, type, existSelfFriend == null ? "null" : existSelfFriend.toString(),
					existDestFriend == null ? "null" : existDestFriend.toString());
			return false;
		} catch (Exception e) {
			logger.error("user_id:{},friendId:{}, type:{} , e:{}", userId, friendId, type, e);
			return false;
		}
	}
}
