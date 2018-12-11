#include "Transform.hpp"

#include <glm\gtc\matrix_transform.hpp>

const glm::vec3 Transform::globalUp(0, 0, 1);
const glm::vec3 Transform::globalDown(0, 0, -1);
const glm::vec3 Transform::globalLeft(-1, 0, 0);
const glm::vec3 Transform::globalRight(1, 0, 0);
const glm::vec3 Transform::globalForward(0, 1, 0);
const glm::vec3 Transform::globalBack(0, -1, 0);

const glm::vec3 Transform::localUp(0, 1, 0);
const glm::vec3 Transform::localDown(0, -1, 0);
const glm::vec3 Transform::localLeft(-1, 0, 0);
const glm::vec3 Transform::localRight(1, 0, 0);
const glm::vec3 Transform::localForward(0, 0, -1);
const glm::vec3 Transform::localBack(0, 0, 1);

Transform::Transform(Engine& engine, uint64_t id) : ComponentInterface(engine, id) {
	INTERFACE_ENABLE(engine, ComponentInterface::serialize)(0);
}

Transform::~Transform() {
	removeParent();
	removeChildren();
}

void Transform::serialize(BaseReflector& reflector) {
	reflector.buffer("transform", "position", &position[0], 3);
	reflector.buffer("transform", "scale", &scale[0], 3);

	uint32_t parentIndex;
	glm::vec3 eulerAngles;

	switch (reflector.mode) {
	case BaseReflector::Out:
		if (_parent) {
			parentIndex = front64(_parent);
			reflector.buffer("transform", "parent", &parentIndex);
		}

		eulerAngles = glm::degrees(glm::eulerAngles(rotation));
		reflector.buffer("transform", "rotation", &eulerAngles[0], 3);
		return;

	case BaseReflector::In:
		if (reflector.buffered("transform", "parent")) {
			reflector.buffer("transform", "parent", &parentIndex);
			_parent = combine32(parentIndex, 1);
		}

		if (reflector.buffered("transform", "rotation")) {
			reflector.buffer("transform", "rotation", &eulerAngles[0], 3);
			rotation = glm::quat(glm::radians(eulerAngles));
		}
		return;
	}
}

void Transform::addChild(uint64_t childId) {
	Transform* newChild = _engine.getComponent<Transform>(childId);

	// if no transform, or already a child of this
	if (!newChild || newChild->_parent == _id)
		return;

	// if child of something else
	if (newChild->_parent)
		newChild->removeParent();

	// set parent
	newChild->_parent = _id;

	// if first child
	if (!_firstChild) {
		newChild->_leftSibling = childId;
		newChild->_rightSibling = childId;
		_firstChild = childId;

		return;
	}

	Transform* firstChild = _engine.getComponent<Transform>(_firstChild);
	Transform* secondChild;

	// if only one sibling
	if (firstChild->_rightSibling == _firstChild)
		secondChild = firstChild;
	else
		secondChild = _engine.getComponent<Transform>(firstChild->_rightSibling);

	// add into linked list after first child   O O O O --> O X O O O
	newChild->_leftSibling = _firstChild;
	newChild->_rightSibling = firstChild->_rightSibling;

	firstChild->_rightSibling = childId;
	secondChild->_leftSibling = childId;
}

void Transform::removeParent() {
	if (!_parent)
		return;

	Transform* parent = _engine.getComponent<Transform>(_parent);

	// if only child
	if (_rightSibling == _id) {
		parent->_firstChild = 0;
		_rightSibling = _id;
		_leftSibling = _id;
	}
	else {
		// set parent
		if (parent->_firstChild == _id)
			parent->_firstChild = _rightSibling;

		Transform* leftSibling = _engine.getComponent<Transform>(_leftSibling);
		Transform* rightSibling;

		// if only one sibling
		if (_leftSibling == _rightSibling)
			rightSibling = leftSibling;
		else
			rightSibling = _engine.getComponent<Transform>(_rightSibling);

		// remove from linked list
		leftSibling->_rightSibling = _rightSibling;
		rightSibling->_leftSibling = _leftSibling;
	}

	_parent = 0;
	_rightSibling = _id;
	_leftSibling = _id;
}

void Transform::removeChildren() {
	if (!_firstChild)
		return;

	uint64_t i = _rightSibling;

	while (i != _id) {
		Transform* sibling = _engine.getComponent<Transform>(_rightSibling);

		i = sibling->_rightSibling;

		sibling->_parent = 0;
		sibling->_leftSibling = sibling->_id;
		sibling->_rightSibling = sibling->_id;
	}

	_firstChild = 0;
}

void Transform::_getChildren(uint64_t id, uint32_t count, std::vector<uint64_t>* ids) const {
	Transform* sibling = _engine.getComponent<Transform>(id);

	if (sibling->_rightSibling == _firstChild)
		ids->resize(count + 1);
	else
		_getChildren(sibling->_rightSibling, count + 1, ids);

	ids->at(count) = sibling->_id;
}

bool Transform::hasChildren() const {
	return _firstChild;
}

void Transform::getChildren(std::vector<uint64_t>* ids) const {
	if (!_firstChild)
		return;

	_getChildren(_firstChild, 0, ids);
}

glm::mat4 Transform::localMatrix() const{
	glm::mat4 matrix;

	matrix = glm::translate(matrix, position);
	matrix *= glm::mat4_cast(rotation);
	matrix = glm::scale(matrix, scale);

	return matrix;
}

glm::mat4 Transform::globalMatrix() const {
	glm::mat4 matrix;

	const Transform* transform = this;

	do {
		matrix = transform->localMatrix() * matrix;
		transform = _engine.getComponent<Transform>(transform->_parent);
	} while (transform);

	return matrix;
}

void Transform::localRotate(const glm::quat& rotate) {
	rotation = rotation * rotate;
}

void Transform::localTranslate(const glm::vec3& translation) {
	position = position + rotation * translation;
}

void Transform::localScale(const glm::vec3 & scaling) {
	scale = scale * scaling;
}

void Transform::globalRotate(const glm::quat& rotate) {
	//if (!parent)
		rotation = rotate * rotation;
	//else
	//	_setRotation(glm::inverse(parent->worldRotation()) * (rotation * worldRotation()));
}

void Transform::globalTranslate(const glm::vec3& translation) {
	//if (!parent)
		position = position + translation;
	//else
	//	_setPosition(_position + glm::inverse(parent->worldRotation()) * translation);
}

void Transform::globalScale(const glm::vec3 & scaling) {
	//if (!parent)
		scale = scale * scaling;
	//else
	//	_setScale(scaling / parent->worldScale());
}