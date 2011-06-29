from .multifitLib import definition_PositionComponent as PositionComponent
from .multifitLib import definition_RadiusComponent as RadiusComponent
from .multifitLib import definition_EllipticityComponent as EllipticityComponent
from .multifitLib import definition_ObjectComponent as ObjectComponent
from .multifitLib import definition_ObjectComponentSet as ObjectComponentSet
from .multifitLib import definition_Frame as Frame
from .multifitLib import definition_FrameSet as FrameSet
from .multifitLib import Definition

from . import multifitLib

multifitLib.Definition.ObjectComponent = ObjectComponent
multifitLib.Definition.Frame = Frame
multifitLib.Definition.ObjectComponentSet = ObjectComponentSet
multifitLib.Definition.FrameSet = FrameSet
multifitLib.Definition.PositionComponent = PositionComponent
multifitLib.Definition.RadiusComponent = RadiusComponent
multifitLib.Definition.EllipticityComponent = EllipticityComponent

PositionComponent.Bounds = multifitLib.detail_CircleConstraint
RadiusComponent.Bounds = multifitLib.detail_MinMaxConstraint
EllipticityComponent.Bounds = multifitLib.detail_CircleConstraint
