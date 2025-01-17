#include "transform_launch.h"
#include "transform_unity.h"

transform_launch::transform_launch()
{
	carryStart();
	m_gdal.gdalRegister();

}

void transform_launch::initialize(U_Transform u_param)
{
	m_param = io_transfer::transfer<U_Transform, U_Transform>(u_param);
	init("gdalTransform", m_param);
}

bool transform_launch::toInformation()
{
	preFiles();
	preInformation();
	carryEnd();
	return true;
}

bool transform_launch::toTransform()
{
	preFiles();
	preTransform();
	carryEnd();
	return true;
}
void transform_launch::preFiles()
{
	std::vector<std::string> inputFotmat{ format_shp,format_tif };
	std::vector<std::string> outputFotmat{ format_shp,format_tif };
	vec_entityTransform = io_composition::getList<entity_transform>(m_param.pBasic, inputFotmat, outputFotmat);
	for (entity_transform& et : vec_entityTransform) {
		et.offsetX = m_param.pTransform.offsetX;
		et.offsetY = m_param.pTransform.offsetY;
		et.offsetZ = m_param.pTransform.offsetZ;
		et.targetSpatial = m_param.pTransform.targetSpatial;
	}
	taskCount = vec_entityTransform.size();
	m_param.printTransform(m_callback);
}

void transform_launch::preInformation()
{
	std::function<bool(int)> func = [&](int taskIndex) {
		if (map_taskQueue.count(taskIndex) > 0) {
			std::vector<int> vec_taskQueue = map_taskQueue[taskIndex];
			for (size_t i = 0; i < vec_taskQueue.size(); i++) {
				int index = vec_taskQueue[i];
				entity_transform& et = vec_entityTransform[index];
				transform_unity unity;
				unity.set(m_param, m_callback);
				unity.information(et);
			}
		}
		return true;
		};
	async(func);
}

void transform_launch::preTransform()
{
	std::function<bool(int)> func = [&](int taskIndex) {
		if (map_taskQueue.count(taskIndex) > 0) {
			std::vector<int> vec_taskQueue = map_taskQueue[taskIndex];
			for (size_t i = 0; i < vec_taskQueue.size(); i++) {
				int index = vec_taskQueue[i];
				entity_transform& et = vec_entityTransform[index];
				transform_unity unity;
				unity.set(m_param, m_callback);
				unity.transform(et);
			}
		}
		return true;
		};
	async(func);
}

