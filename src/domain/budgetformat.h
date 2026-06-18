#pragma once

#include <QString>
#include <QVector>

class QJsonObject;

struct BudgetSection {
	QString name;
	bool locked = false;
	QVector<BudgetSection> children;
};

class BudgetFormat {
public:
	BudgetFormat();

	QVector<BudgetSection>& roots();
	const QVector<BudgetSection>& roots() const;

	bool loadFromFile(const QString& filePath);
	bool saveToFile(const QString& filePath) const;

private:
	void resetToDefaults();

	static QJsonObject sectionToJson(const BudgetSection& section);
	static BudgetSection sectionFromJson(const QJsonObject& object);

	QVector<BudgetSection> rootSections;
};
