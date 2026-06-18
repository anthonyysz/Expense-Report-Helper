#include "budgetformat.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

BudgetFormat::BudgetFormat()
{
	resetToDefaults();
}

QVector<BudgetSection>& BudgetFormat::roots()
{
	return rootSections;
}

const QVector<BudgetSection>& BudgetFormat::roots() const
{
	return rootSections;
}

bool BudgetFormat::loadFromFile(const QString& filePath)
{
	QFile file(filePath);

	if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
		return false;
	}

	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);

	if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
		return false;
	}

	const QJsonArray rootsArray = document.object().value("roots").toArray();
	QVector<BudgetSection> loadedRoots;

	for (const QJsonValue& value : rootsArray) {
		if (value.isObject()) {
			loadedRoots.append(sectionFromJson(value.toObject()));
		}
	}

	if (loadedRoots.size() != 2) {
		return false;
	}

	loadedRoots[0].name = "Income";
	loadedRoots[0].locked = true;
	loadedRoots[1].name = "Expenses";
	loadedRoots[1].locked = true;

	rootSections = loadedRoots;
	return true;
}

bool BudgetFormat::saveToFile(const QString& filePath) const
{
	QJsonArray rootsArray;

	for (const BudgetSection& section : rootSections) {
		rootsArray.append(sectionToJson(section));
	}

	QJsonObject rootObject;
	rootObject["roots"] = rootsArray;

	QFile file(filePath);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}

	file.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
	return true;
}

void BudgetFormat::resetToDefaults()
{
	BudgetSection income;
	income.name = "Income";
	income.locked = true;
	income.children = {
		BudgetSection{ "Earned", false, {} },
		BudgetSection{ "Received", false, {} }
	};

	BudgetSection expenses;
	expenses.name = "Expenses";
	expenses.locked = true;
	expenses.children = {
		BudgetSection{ "Necessities", false, {} },
		BudgetSection{ "Food", false, {} },
		BudgetSection{ "Other", false, {} }
	};

	rootSections = {
		income,
		expenses
	};
}

QJsonObject BudgetFormat::sectionToJson(const BudgetSection& section)
{
	QJsonArray childrenArray;

	for (const BudgetSection& child : section.children) {
		childrenArray.append(sectionToJson(child));
	}

	QJsonObject object;
	object["name"] = section.name;
	object["locked"] = section.locked;
	object["children"] = childrenArray;

	return object;
}

BudgetSection BudgetFormat::sectionFromJson(const QJsonObject& object)
{
	BudgetSection section;
	section.name = object.value("name").toString();
	section.locked = object.value("locked").toBool(false);

	for (const QJsonValue& value : object.value("children").toArray()) {
		if (value.isObject()) {
			section.children.append(sectionFromJson(value.toObject()));
		}
	}

	return section;
}
