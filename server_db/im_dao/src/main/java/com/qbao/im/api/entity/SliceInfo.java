/**
 * 
 */
package com.qbao.im.api.entity;

/**
 * 分库分一信息
 * @author andersen
 *
 */
public class SliceInfo {
	private String datasourceName;
	private String tableName;
	public String getDatasourceName() {
		return datasourceName;
	}
	public void setDatasourceName(String datasourceName) {
		this.datasourceName = datasourceName;
	}
	public String getTableName() {
		return tableName;
	}
	public void setTableName(String tableName) {
		this.tableName = tableName;
	}
	@Override
	public String toString(){
		return datasourceName + "." + tableName;
	}
	
}
