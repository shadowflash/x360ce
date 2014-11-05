﻿namespace x360ce.App
{
	public enum ForceFeedBackType
	{
		/// <summary>Old constant force (like in 3.1.4.1)</summary>
		Default = 0,
		/// <summary>EJocys method (from rev 150)</summary>
		PeriodicForWheels = 1,
		/// <summary>New constant force</summary>
		Constant = 2,
	}
}
