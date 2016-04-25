#pragma once
#include "typedefs.hpp"
#include <openrave/openrave.h>
#include "macros.h"
#include <iostream>
namespace trajopt {


class TRAJOPT_API Configuration {
public:
  
  virtual void SetDOFValues(const DblVec& dofs) = 0;
  virtual void GetDOFLimits(DblVec& lower, DblVec& upper) const = 0;
  virtual DblVec GetDOFValues() = 0;
  virtual int GetDOF() const = 0;
  virtual int GetAffineDOF() const = 0;
  virtual OpenRAVE::EnvironmentBasePtr GetEnv() = 0;
  virtual DblMatrix PositionJacobian(int link_ind, const OR::Vector& pt) const = 0;
  virtual DblMatrix RotationJacobian(int link_ind, const OR::Vector& rot) const = 0;
  virtual bool DoesAffect(const KinBody::Link& link) = 0;
  virtual vector<OpenRAVE::KinBodyPtr> GetBodies() = 0;
  virtual std::vector<KinBody::LinkPtr> GetAffectedLinks() = 0;
  virtual void GetAffectedLinks(std::vector<KinBody::LinkPtr>& links, bool only_with_geom, vector<int>& link_inds) = 0;
  virtual DblVec RandomDOFValues() = 0;
  virtual ~Configuration() {}
  struct Saver {
    virtual ~Saver(){}
  };
  typedef boost::shared_ptr<Saver> SaverPtr;
  struct GenericSaver : public Saver {
    DblVec dofvals;
    Configuration* parent;
    GenericSaver(Configuration* _parent) : dofvals(_parent->GetDOFValues()), parent(_parent) {}
    ~GenericSaver() {
      parent->SetDOFValues(dofvals);
    }
  }; // inefficient
  
  virtual SaverPtr Save() {
    return SaverPtr(new GenericSaver(this));
  }


};
typedef boost::shared_ptr<Configuration> ConfigurationPtr;

/**
Stores an OpenRAVE kinbody and the affine degrees of freedom
*/
class TRAJOPT_API KinBodyAndDOF : public Configuration {
public:
  KinBodyAndDOF(OR::KinBodyPtr _kinbody, int _affinedofs=0, const OR::Vector _rotationaxis=OR::Vector(0,0,1)) :
    m_kinbody(_kinbody), m_affinedofs(_affinedofs), m_rotationaxis(_rotationaxis) {}

  void SetDOFValues(const DblVec& dofs);
  DblVec GetDOFValues();
  void GetDOFLimits(DblVec& lower, DblVec& upper) const;
  int GetDOF() const;
  int GetAffineDOF() const;
  OpenRAVE::EnvironmentBasePtr GetEnv() {return m_kinbody->GetEnv();};
  DblMatrix PositionJacobian(int link_ind, const OR::Vector& pt) const;
  DblMatrix RotationJacobian(int link_ind, const OR::Vector& rot) const {PRINT_AND_THROW("Not implemented!");};
  vector<OpenRAVE::KinBodyPtr> GetBodies();
  bool DoesAffect(const KinBody::Link& link);
  std::vector<KinBody::LinkPtr> GetAffectedLinks();
  void GetAffectedLinks(std::vector<KinBody::LinkPtr>& links, bool only_with_geom, vector<int>& link_inds);
  DblVec RandomDOFValues() {PRINT_AND_THROW("Not implemented!");};

  struct KinBodySaver : public Saver {
    OpenRAVE::KinBody::KinBodyStateSaver saver;
    KinBodySaver(OpenRAVE::KinBodyPtr m_kinbody) : saver(m_kinbody) {}
  };
  SaverPtr Save() {
    return SaverPtr(new KinBodySaver(m_kinbody));
  }
  
protected:
  OpenRAVE::KinBodyPtr m_kinbody;
  int m_affinedofs;
  OR::Vector m_rotationaxis;
};
typedef boost::shared_ptr<KinBodyAndDOF> KinBodyAndDOFPtr;

/**
Stores an OpenRAVE robot and the active degrees of freedom
*/
class TRAJOPT_API RobotAndDOF : public KinBodyAndDOF {
public:
  RobotAndDOF(OR::KinBodyPtr _robot, const IntVec& _joint_inds, int _affinedofs=0, const OR::Vector _rotationaxis=OR::Vector(0,0,1)) :
    KinBodyAndDOF(_robot, _affinedofs, _rotationaxis), m_joint_inds(_joint_inds) {}

  void SetDOFValues(const DblVec& dofs);
  void GetDOFLimits(DblVec& lower, DblVec& upper) const;
  DblVec GetDOFValues();
  int GetDOF() const;
  IntVec GetJointIndices() const {return m_joint_inds;}
  DblMatrix PositionJacobian(int link_ind, const OR::Vector& pt) const;
  DblMatrix RotationJacobian(int link_ind, const OR::Vector& rot) const;
  OR::RobotBasePtr GetRobot() const {return boost::dynamic_pointer_cast<RobotBase>(m_kinbody);}
  bool DoesAffect(const KinBody::Link& link);
  void GetAffectedLinks(std::vector<KinBody::LinkPtr>& links, bool only_with_geom, vector<int>& link_inds);
  DblVec RandomDOFValues();
  void SetRobotActiveDOFs();
  
protected:
  IntVec m_joint_inds;
};
typedef boost::shared_ptr<RobotAndDOF> RobotAndDOFPtr;

}
