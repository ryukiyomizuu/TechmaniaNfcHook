// This file mirrors TECHMANIA's integration contract tests. It is included in
// the repository as executable documentation; the game project supplies
// ProfileManager, ProfileData, ThemeProfileApi, and Unity's NUnit runner.
using System.Reflection;
using NUnit.Framework;

public class ProfileCredentialFlowContractTests
{
    [Test]
    public void ThemeApiOwnsCredentialAccessInsteadOfThemeCallingTheDll()
    {
        TypeInfo type = typeof(ThemeApi.ThemeProfileApi).GetTypeInfo();
        Assert.That(type.GetMethod("pollForCredential"), Is.Not.Null);
        Assert.That(type.GetMethod("pendingCredentialKind"), Is.Not.Null);
        Assert.That(type.GetMethod("loginWithPendingCredential"), Is.Not.Null);
        Assert.That(type.GetMethod("createProfileWithPendingCredential"), Is.Not.Null);
        Assert.That(type.GetMethod("beginCredentialLink"), Is.Not.Null);
        Assert.That(type.GetMethod("linkPendingCredential"), Is.Not.Null);
        Assert.That(type.GetMethod("cancelCredentialLink"), Is.Not.Null);
    }
}
